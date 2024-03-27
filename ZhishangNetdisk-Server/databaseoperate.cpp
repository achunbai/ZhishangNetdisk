#include "databaseoperate.h"

DatabaseOperate &DatabaseOperate::GetInstance()
{
    static DatabaseOperate instance;
    return instance;
}

DatabaseOperate::~DatabaseOperate()
{
    // 析构函数中关闭对数据库的连接
    MySQLDatabase.close();
}

void DatabaseOperate::ConnectDB()
{
    // 设置MySQL数据库的参数，这里使用的是MySQL8

    // setHostName会自动识别是不是域名
    MySQLDatabase.setHostName(MySQLHost);
    MySQLDatabase.setDatabaseName(MySQLDatabaseName);
    MySQLDatabase.setPort(MySQLPort);
    MySQLDatabase.setUserName(MySQLUsername);
    MySQLDatabase.setPassword(MySQLPassword);
    // 输出连接情况，方便调试
    if(MySQLDatabase.open())
    {
        qDebug() << "数据库连接成功";
    }
    else
    {
        // 使用QSqlError类型，但是没有包含QSqlError的定义。
        // QSqlError类型是在QSqlError头文件中定义的，所以需要在代码中包含这个头文件。
        // error: calling 'lastError' with incomplete return type 'QSqlError'
        qDebug() << "数据库连接失败，错误为：\n" << MySQLDatabase.lastError();
        // QSqlError对象不能直接转换为字符串，所以不能直接将它与字符串进行连接
        // QSqlError类有一个text()成员函数，可以返回一个描述错误的字符串
        // error: invalid operands to binary expression ('const char[26]' and 'QSqlError')
        qDebug() << QSqlDatabase::drivers();
        exit(1);
    }
}

bool DatabaseOperate::ProcessRegister(const char *UserName, const char *Password, int& UID)
{
    // 检查用户名和密码是否为空
    if(UserName == NULL || Password == NULL)
    {
        qDebug() << "DatabaseOperate::ProcessRegister  UserName == NULL || Password == NULL";
        return false;
    }

    // 检查添加的用户是否存在
    // %1，%2表示的是第几个参数，后面加arg(参数内容)按顺序添加
    QString sql = QString("select id from user_info where username = '%1'").arg(UserName);
    QSqlQuery q;

    // 语句执行失败和查找到了用户名已经存在，那么不继续执行，注册失败
    // exec用于执行sql语句，next用于将查询到的结果集一个一个取出
    if(!q.exec(sql) || q.next())
    {
        qDebug() << "DatabaseOperate::ProcessRegister  !q.exec(sql) || q.next()";
        return false;
    }

    // 添加用户，构建添加用户的SQL语句
    // 不要写两个arg，在一个arg内写多个参数即可，防止重复调用函数
    sql = QString("insert into user_info (username, password) values ('%1', '%2')").arg(UserName, Password);
    qDebug() << "用户注册sql语句：" << sql;
    // 返回SQL语句执行结果
    bool ret = q.exec(sql);
    // 查询一遍获取UID
    sql = QString("select id from user_info where username = '%1'").arg(UserName);
    if(q.exec(sql) && q.next())
        UID = q.value(0).toInt();
    else
        return false;
    return ret;
}

bool DatabaseOperate::ProcessLogin(const char *UserName, const char *Password, int& UID)
{
    // 检查用户名和密码是否为空
    if(UserName == NULL || Password == NULL)
    {
        qDebug() << "DatabaseOperate::ProcessLogin  UserName == NULL || Password == NULL";
        return false;
    }

    // 检查添加的用户是否存在
    // %1，%2表示的是第几个参数，后面加arg(参数内容)按顺序添加
    QString sql = QString("select id from user_info where username = '%1' and password = '%2'").arg(UserName).arg(Password);
    QSqlQuery q;

    // 语句执行失败和查找到了用户名已经存在，那么不继续执行，注册失败
    // exec用于执行sql语句，next用于将查询到的结果集一个一个取出
    if(!q.exec(sql) || !q.next())
    {
        qDebug() << "DatabaseOperate::ProcessLogin  !q.exec(sql) || !q.next()";
        return false;
    }
    // 获取UID
    UID = q.value(0).toInt();
    // 修改用户登录状态，构建SQL语句
    // 使用多参数版本的arg()函数，这样可以避免多次创建临时的QString对象，提高代码的效率
    // .arg(UserName).arg(Password)   ->   .arg(UserName, Password)
    sql = QString("update user_info set isOnline = 1 where username = '%1' and password = '%2'").arg(UserName, Password);
    qDebug() << "用户登录sql语句：" << sql;
    // 返回SQL语句执行结果
    // 返回后在TcpSocket中去保存当前用户名
    return q.exec(sql);
}

void DatabaseOperate::ProcessOffline(const char *UserName)
{
    if(UserName == NULL || *UserName == 0)
    {
        qDebug() << "DatabaseOperate::ProcessOffline  UserName == NULL";
        return;
    }

    // 构建sql语句
    QString sql = QString("update user_info set isOnline = 0 where username = '%1'").arg(UserName);
    qDebug() << "处理离线sql语句：" << sql;
    QSqlQuery q;
    // 执行sql语句
    qDebug() << "DatabaseOperate::ProcessOffline  q.exec(sql)：" << q.exec(sql);
}

int DatabaseOperate::ProcessSearchUser(const char *UserName)
{
    if(UserName == NULL)
    {
        qDebug() << "DatabaseOperate::ProcessSearchUser  UserName == NULL";
        return -1;
    }

    QString sql = QString("select isOnline from user_info where username = '%1'").arg(UserName);
    QSqlQuery q;
    q.exec(sql);
    // 使用q.value(0).toInt()获取查询结果
    // q.value(0)表示的是结果集的第一个字段，toInt()表示将这个字段转换为整数
    // 注意q.value()获取到的是QVariant类型，所以需要调用toInt()函数将其转换为整数
    if(q.next())
    {
        uint isOnline = q.value(0).toUInt();
        return isOnline;
    }
    else
        return -1;
}

QStringList DatabaseOperate::ProcessOnlineUsersList()
{
    QString sql = QString("select username from user_info where isOnline = 1");
    QSqlQuery q;
    q.exec(sql);
    QStringList OnlineUsersList;
    // 遍历获取的结果，写入一个QStringList
    // q.value(0)取第一列，toString转成QString然后存储在QStringList中
    while(q.next())
        OnlineUsersList.append(q.value(0).toString());
    return OnlineUsersList;
}

int DatabaseOperate::ProcessFriendSearch(const char *CurrentUserName, const char *TargetUserName)
{
    if(CurrentUserName == NULL || TargetUserName == NULL)
        // 返回-1，错误
        return -1;
    QString sql = QString(R"(select id from friend_info where (
                                     (
                                         user_id = (select id from user_info where username = '%1')
                                             and
                                         friend_id = (select id from user_info where username = '%2')
                                         )
                                         or
                                     (
                                         friend_id = (select id from user_info where username = '%1')
                                             and
                                         user_id = (select id from user_info where username = '%2')
                                         )
                                     );)").arg(CurrentUserName, TargetUserName);
    qDebug() << "查询好友状态sql语句：" << sql;
    QSqlQuery q;
    q.exec(sql);
    if(q.next())
        // 返回-2，已经是好友
        return -2;
    else
    {
        // 返回1用户在线，0为用户离线
        return ProcessSearchUser(TargetUserName);
    }
}

bool DatabaseOperate::ProcessAddFriend(const char *CurrentUserName, const char *TargetUserName)
{
    if(CurrentUserName == NULL || TargetUserName == NULL)
        return false;
    // 构建处理添加好友的sql语句，同时查询当前用户id和被添加好友的id
    QString sql = QString(R"(insert into friend_info(user_id, friend_id) values(
                                                   (select id from user_info where username = '%1'),
                                                   (select id from user_info where username = '%2')
                                                  );)").arg(CurrentUserName, TargetUserName);
    qDebug() << "添加好友sql语句：" << sql;
    QSqlQuery q;
    return q.exec(sql);
}

QStringList DatabaseOperate::ProcessFriendList(const char *UserName, QVector<int>& Online, bool& status)
{
    QStringList FriendList;
    if(UserName == NULL)
    {
        status = false;
        return FriendList;
    }
    // 构建查询好友列表的SQL语句，用户可能在user_id，也可能在friend_id中
    QString sql = QString(R"(select username, isOnline from user_info where id = (
                                (select friend_id from friend_info where user_id = (select id from user_info where username = '%1'))
                                    union
                                (select user_id from friend_info where friend_id = (select id from user_info where username = '%1'))
                                );)").arg(UserName);
    qDebug() << "查询好友列表sql语句：" << sql;
    QSqlQuery q;
    status = q.exec(sql);
    while(q.next())
    {
        FriendList.append(q.value(0).toString());
        Online.push_back(q.value(1).toInt());
    }
    return FriendList;
}

bool DatabaseOperate::ProcessDeleteFriend(const char *CurrentUserName, const char *TargetUserName)
{
    if(CurrentUserName == NULL || TargetUserName == NULL)
        return false;
    // 构建删除好友的SQL语句，注意需要正反查询
    QString sql = QString(R"(delete from friend_info where(
    (
        user_id = (select id from user_info where username = '%1')
            and
        friend_id = (select id from user_info where username = '%2')
        )
        or
    (
        friend_id = (select id from user_info where username = '%1')
            and
        user_id = (select id from user_info where username = '%2')
        )
                                 );)").arg(CurrentUserName, TargetUserName);
    qDebug() << "删除好友sql语句：" << sql;
    QSqlQuery q;
    return q.exec(sql);
}

void DatabaseOperate::SetMySQLHost(QString MySQLHost)
{
    this->MySQLHost = MySQLHost;
}

void DatabaseOperate::SetMySQLPort(qint16 MySQLPort)
{
    this->MySQLPort = MySQLPort;
}

void DatabaseOperate::SetMySQLDatabase(QString MySQLDatabaseName)
{
    this->MySQLDatabaseName = MySQLDatabaseName;
}

void DatabaseOperate::SetMySQLUsername(QString MySQLUsername)
{
    this->MySQLUsername = MySQLUsername;
}

void DatabaseOperate::SetMySQLPassword(QString MySQLPassword)
{
    this->MySQLPassword = MySQLPassword;
}

DatabaseOperate::DatabaseOperate(QObject *parent)
    : QObject{parent}
{
    // 定义连接的数据库类型，这里使用的是MySQL
    // 连接成功会返回一个数据库对象(QSqlDatabase)
    MySQLDatabase = QSqlDatabase::addDatabase("QMYSQL");
}
