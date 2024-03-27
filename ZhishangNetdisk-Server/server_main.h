#ifndef SERVER_MAIN_H
#define SERVER_MAIN_H

#include <QTcpSocket>
#include <QWidget>
#include <QFile>
#include <QDebug>
#include <QMessageBox>

#include "zhishangtcpserver.h"
#include "databaseoperate.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Server_Main;
}
QT_END_NAMESPACE

class Server_Main : public QWidget
{
    Q_OBJECT

public:
    ~Server_Main();
    static Server_Main& GetInstance();
    QString GetUserFilePath();

    // 和客户端一样，添加载入配置文件方法
    void LoadConfig();

private:
    Ui::Server_Main *ui;

    Server_Main(QWidget *parent = nullptr);
    Server_Main(const Server_Main& instance) = delete;
    Server_Main& operator=(const Server_Main&) = delete;

    // 服务器端口号和IP
    QString ServerIP;
    // QString ServerHost;
    quint16 ServerPort;

    // 用户文件路径
    QString UserFilePath;

};
#endif // SERVER_MAIN_H
