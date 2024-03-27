#include "server_main.h"
#include "ui_server_main.h"

#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>

Server_Main::Server_Main(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Server_Main)
{
    ui->setupUi(this);

    // 读取配置文件
    LoadConfig();

    // 获取ZhishangTcpServer实例，监听连接
    ZhishangTcpServer::GetInstance().listen(QHostAddress(ServerIP), ServerPort);

    // QHostAddress address;
    // if (!address.setAddress(ServerHost)) {
    //     // 如果 ServerHost 不是一个有效的 IP 地址，那么我们将其视为一个域名并进行解析
    //     QHostInfo info = QHostInfo::fromName(ServerHost);
    //     QList<QHostAddress> addresses = info.addresses();
    //     if (!addresses.isEmpty()) {
    //         // 使用第一个 IP 地址
    //         address = addresses.first();
    //     } else {
    //         qDebug() << "域名解析失败";
    //         return;
    //     }
    // }
    // ZhishangTcpServer::GetInstance().listen(address, ServerPort);
    DatabaseOperate::GetInstance().ConnectDB();
}

Server_Main::~Server_Main()
{
    delete ui;
}

Server_Main &Server_Main::GetInstance()
{
    static Server_Main instance;
    return instance;
}

QString Server_Main::GetUserFilePath()
{
    return UserFilePath;
}

// void Server_Main::LoadConfig()
// {
//     // 从配置文件中读取配置
//     QFile file(":/config/server.config");
//     qDebug() << "读取配置文件";
//     if(!file.exists())
//     {
//         qDebug() << "配置文件不存在";
//         // QMessageBox::critical(this, "打开配置文件错误", "配置文件不存在");
//     }
//     else if(!(file.permissions() & QFileDevice::ReadUser))
//     {
//         qDebug() << "没有读取配置文件的权限";
//         // QMessageBox::critical(this, "打开配置文件错误", "没有读取配置文件的权限");
//     }
//     else if(file.open(QIODevice::ReadOnly))
//     {
//         // 如果打开成功，读取所有文件内容
//         // QByteArray ConfigDataRaw = file.readAll();
//         // 将读取出的数据转换成字符串
//         // QString ConfigDataString = QString(ConfigDataRaw);

//         // 使用QTextStream来读取文件，它会自动处理不同平台的换行符问题
//         QTextStream in(&file);
//         QString ConfigDataString = in.readAll();
//         qDebug() << ConfigDataString;

//         // 注意：换行符在不同的系统中可能不一样
//         // Windows中，文件内每行以\r\n结尾，直接以\r\n进行分割，获取端口和IP
//         // MacOS中，文件内每行以\n结尾，需要以\n进行分割，获取端口和IP
//         // QStringList ConfigDataList = ConfigDataString.split("\r\n");
//         // 为了避免这个问题，可以使用QTextStream来读取文件，它会自动处理不同平台的换行符问题
//         // 跳转到QByteArray ConfigDataRaw = file.readAll();开始修改
//         // 这样都会以\n作为每行的结尾
//         QStringList ConfigDataList = ConfigDataString.split("\n");

//         // 检查ConfigDataList是否有足够的元素
//         if(ConfigDataList.size() >= 3)
//         {
//             // 配置文件中的IP在第一行，端口在第二行，IP为字符串类型，端口为quint16类型，需要进行转换、
//             ServerIP = ConfigDataList.at(0);
//             ServerPort = ConfigDataList.at(1).toUShort();
//             UserFilePath = ConfigDataList.at(2);
//             // 添加一个提示框，显示IP和端口，端口不转换为字符串只会显示一个-
//             // 在Qt中，可以使用QString::number函数将数字转换为字符串。
//             // QString numStr = QString::number(num);
//             // QMessageBox::information(this, "打开配置成功", "IP：" + ServerIP + "\n" + "端口：" + QString::number(ServerPort));
//             // 好像直接用分割后的源数据的好一点吧，这样就不需要再把数字转为字符串
//             qDebug() << "打开配置文件成功：\nIP：" + ServerIP + "\n" + "端口：" + ConfigDataList.at(1);
//             // QMessageBox::information(this, "打开配置文件成功", "IP：" + ServerIP + "\n" + "端口：" + ConfigDataList.at(1));
//         }
//         else
//         {
//             /*
//              * QMessageBox::critical是Qt库中的一个静态函数，用于显示一个错误消息对话框。这个函数接受三个参数：
//              * this：这是一个指向父窗口的指针。如果你在一个成员函数中调用QMessageBox::critical，你通常会使用this作为父窗口。这样，当消息框被显示时，它会阻止用户与父窗口进行交互，直到用户关闭消息框。
//              * "打开配置"：这是消息框的标题。
//              * "配置文件格式错误"：这是在消息框中显示的文本。
//              * 所以，QMessageBox::critical(this, "打开配置", "配置文件格式错误");这行代码的意思是，显示一个错误消息框，标题为"打开配置"，内容为"配置文件格式错误"。
//              */
//             qDebug() << "配置文件格式错误";
//             // QMessageBox::critical(this, "打开配置文件错误", "配置文件格式错误");
//         }
//     }
//     else
//     {
//         qDebug() << "打开配置失败";
//         // QMessageBox::critical(this, "打开配置错误", "打开配置失败");
//     }
// }

void Server_Main::LoadConfig()
{
    // 从配置文件中读取配置
    QFile file(":/config/server.config");
    qDebug() << "读取配置文件";
    if(!file.exists())
    {
        qDebug() << "配置文件不存在！";
        exit(1);
    }
    else if(!(file.permissions() & QFileDevice::ReadUser))
    {
        qDebug() << "没有读取配置文件的权限";
    }
    else if(file.open(QIODevice::ReadOnly))
    {
        // 打开成功，读取全部内容
        QByteArray ConfigDataRaw = file.readAll();

        // 将读取出的数据转换为JSON文档
        QJsonDocument ConfigDoc = QJsonDocument::fromJson(ConfigDataRaw);

        if(ConfigDoc.isNull())
        {
            qDebug() << "配置文件格式错误，创建一个新的配置文件模板";
            file.close();

            QFile tamplate("./Config_Tamplate.config");
            // 创建新的配置文件
            if(tamplate.open(QIODevice::WriteOnly))
            {
                // 创建JSON对象，用于存储配置模板
                QJsonObject ConfigTamplateObj;
                ConfigTamplateObj["ServerIP"] = "127.0.0.1";
                ConfigTamplateObj["ServerPort"] = 9000;
                ConfigTamplateObj["UserFilePath"] = "./userfiles_by_id";
                ConfigTamplateObj["MySQLHost"] = "localhost";
                ConfigTamplateObj["MySQLPort"] = 3306;
                ConfigTamplateObj["MySQLDatabase"] = "net_disk";
                ConfigTamplateObj["MySQLUsername"] = "";
                ConfigTamplateObj["MySQLPassword"] = "";

                // 将JSON对象转换为JSON文档
                QJsonDocument ConfigTamplateDoc(ConfigTamplateObj);

                // 将JSON文档写入文件
                tamplate.write(ConfigTamplateDoc.toJson());

                tamplate.close();
            }
            else
            {
                qDebug() << "创建配置文件模板失败！";
            }
            exit(1);
        }
        else
        {
            // 获取JSON对象
            QJsonObject ConfigObj = ConfigDoc.object();

            // 读取每项配置
            ServerIP = ConfigObj["ServerIP"].toString();
            ServerPort = ConfigObj["ServerPort"].toInt();
            UserFilePath = ConfigObj["UserFilePath"].toString();
            DatabaseOperate::GetInstance().SetMySQLHost(ConfigObj["MySQLHost"].toString());
            DatabaseOperate::GetInstance().SetMySQLPort(ConfigObj["MySQLPort"].toInt());
            DatabaseOperate::GetInstance().SetMySQLDatabase(ConfigObj["MySQLDatabase"].toString());
            DatabaseOperate::GetInstance().SetMySQLUsername(ConfigObj["MySQLUsername"].toString());
            DatabaseOperate::GetInstance().SetMySQLPassword(ConfigObj["MySQLPassword"].toString());

            qDebug() << "ServerIP " << ServerIP;
            qDebug() << "ServerPort " << ServerPort;
            qDebug() << "UserFilePath " << UserFilePath;
            qDebug() << "MySQLHost " << ConfigObj["MySQLHost"].toString();
            qDebug() << "MySQLPort " << ConfigObj["MySQLPort"].toInt();
            qDebug() << "MySQLDatabase " << ConfigObj["MySQLDatabase"].toString();
            qDebug() << "MySQLUsername " << ConfigObj["MySQLUsername"].toString();
            qDebug() << "MySQLPassword " << ConfigObj["MySQLPassword"].toString();
            file.close();
        }
    }
    else
    {
        qDebug() << "打开配置失败";
        exit(-1);
    }

}
