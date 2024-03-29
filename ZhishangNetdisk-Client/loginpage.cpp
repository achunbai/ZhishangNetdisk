#include "loginpage.h"
#include "ui_loginpage.h"

#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostInfo>

// 构造函数
LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginPage)
{
    ui->setupUi(this);
    // 需要在构造函数中调用载入配置文件函数
    LoadConfig();

    // 链接成功时，ClientSocket会发出一个成功的信号，为QTcpSocket::connected
    // 通过这个信号可以知道其是否连接成功，可以根据这个信号添加一个对应的槽函数来知道什么时候连接上了服务器
    // [static] QMetaObject::Connection QObject::connect(const QObject *sender, const char *signal, const QObject *receiver, const char *method, Qt::ConnectionType type = Qt::AutoConnection)
    // Creates a connection of the given type from the signal in the sender object to the method in the receiver object. Returns a handle to the connection that can be used to disconnect it later.
    // 需要创建一个槽函数来处理连接上的情况
    // 跳转到头文件创建槽函数
    // 注意ShowConnect在这里只是引用它，不是调用它，所以不需要加上括号
    connect(&ClientSocket, &QTcpSocket::connected, this, &LoginPage::ShowConnect);

    // 连接服务器
    // connectToHost(QHostAddress(地址字符串), 数字端口号)
    // QHostAddress用来将地址字符串转换为可以识别的IP地址(QHostAddress类型)
    // QHostAddress::QHostAddress(const QString &address)
    //     Constructs an IPv4 or IPv6 address based on the string address (e.g., "127.0.0.1").
    // ClientSocket.connectToHost(QHostAddress(ServerIP), ServerPort);

    QHostAddress address;
    if (!address.setAddress(ServerHost)) {
        // 如果 ServerHost 不是一个有效的 IP 地址，那么我们将其视为一个域名并进行解析
        QHostInfo info = QHostInfo::fromName(ServerHost);
        QList<QHostAddress> addresses = info.addresses();
        if (!addresses.isEmpty()) {
            // 使用第一个 IP 地址
            address = addresses.first();
        } else {
            qDebug() << "域名解析失败";
            return;
        }
    }
    ClientSocket.connectToHost(address, ServerPort);
    // 跳转到13行，编写connect方法来连接信号槽

    // 连接ReceiveMsg函数和接收到socket中可以读取的信号
    connect(&ClientSocket, &QTcpSocket::readyRead, this, &LoginPage::ReceiveMsg);

    MsgProcessor = new MsgHandler;

}

LoginPage::~LoginPage()
{
    delete ui;
    delete MsgProcessor;
}

// void LoginPage::LoadConfig()
// {
//     // IP和端口号应该放在配置文件中的
//     // ServerIP = "127.0.0.1";
//     // ServerPort = 11451;
//     // 从配置文件中读取配置
//     QFile file(":/config/client.config");
//     if(!file.exists())
//     {
//         QMessageBox::critical(this, "打开配置文件错误", "配置文件不存在");
//     }
//     else if(!(file.permissions() & QFileDevice::ReadUser))
//     {
//         QMessageBox::critical(this, "打开配置文件错误", "没有读取配置文件的权限");
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
//         if(ConfigDataList.size() >= 2)
//         {
//             // 配置文件中的IP在第一行，端口在第二行，IP为字符串类型，端口为quint16类型，需要进行转换、
//             ServerIP = ConfigDataList.at(0);
//             ServerPort = ConfigDataList.at(1).toUShort();
//             // 添加一个提示框，显示IP和端口，端口不转换为字符串只会显示一个-
//             // 在Qt中，可以使用QString::number函数将数字转换为字符串。
//             // QString numStr = QString::number(num);
//             // QMessageBox::information(this, "打开配置成功", "IP：" + ServerIP + "\n" + "端口：" + QString::number(ServerPort));
//             // 好像直接用分割后的源数据的好一点吧，这样就不需要再把数字转为字符串
//             QMessageBox::information(this, "打开配置文件成功", "IP：" + ServerIP + "\n" + "端口：" + ConfigDataList.at(1));
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
//             QMessageBox::critical(this, "打开配置文件错误", "配置文件格式错误");
//         }
//     }
//     else
//     {
//         QMessageBox::critical(this, "打开配置错误", "打开配置失败");
//     }
// }

void LoginPage::LoadConfig()
{
    // 从配置文件中读取配置
    QFile file(":/config/client.config");
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
                ConfigTamplateObj["ServerHost"] = "127.0.0.1";
                ConfigTamplateObj["ServerPort"] = 9000;

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
            ServerHost = ConfigObj["ServerHost"].toString();
            ServerPort = ConfigObj["ServerPort"].toInt();

            qDebug() << "ServerHost " << ServerHost;
            qDebug() << "ServerPort " << ServerPort;
            file.close();
        }
    }
    else
    {
        qDebug() << "打开配置失败";
        exit(-1);
    }

}

LoginPage &LoginPage::GetInstance()
{
    // 静态局部变量，只能被初始化一次，生命周期从定义开始到程序运行结束
    // 因其在函数结束运行后不会被销毁，所以采用静态局部变量
    static LoginPage instance;
    // C++ 11后静态局部变量是线程安全的
    //  线程安全是指在多线程环境下，一个函数、函数库在同时被多个线程访问时，仍能正确地返回结果的特性。
    //  如果一个函数是线程安全的，那么多个线程可以同时调用这个函数，而不会导致错误或数据不一致。
    //  在C++11及其后续版本中，静态局部变量的初始化是线程安全的。
    //  这是因为C++标准规定，静态局部变量的初始化必须是线程安全的。
    //  具体来说，如果多个线程同时尝试初始化同一个静态局部变量，只有一个线程会执行初始化，其他线程会等待，直到该变量被初始化为止。
    //  这种特性使得静态局部变量成为实现单例模式的理想选择，因为它们可以确保在多线程环境下只被初始化一次。
    return instance;
}

QTcpSocket& LoginPage::GetSocket()
{
    return ClientSocket;
}

QString &LoginPage::GetCurrentUser()
{
    return CurrentUser;
}

void LoginPage::ProcessPDU(PDU *rPDU)
{
    // 根据接收到的协议选择处理函数
    // 记得写break！！
    switch(rPDU->MsgType)
    {
        case MSG_TYPE_REGISTER_RESPOND:
            MsgProcessor->ProcessRegisterRespond(rPDU);
            break;

        case MSG_TYPE_LOGIN_RESPOND:
            MsgProcessor->ProcessLoginRespond(rPDU);
            break;

        case MSG_TYPE_SEARCH_USER_RESPOND:
            MsgProcessor->ProcessSearchUserRespond(rPDU);
            break;

        case MSG_TYPE_SHOW_ONLINE_USERS_RESPOND:
            MsgProcessor->ProcessShowOnlieneUserRespond(rPDU);
            break;

        case MSG_TYPE_ADD_FRIEND_REQUEST:
            MsgProcessor->ProcessAddFriendRequest(rPDU);
            break;

        case MSG_TYPE_ADD_FRIEND_RESPOND:
            MsgProcessor->ProcessAddFriendRespond(rPDU);
            break;

        case MSG_TYPE_ADD_FRIEND_RESULT:
            MsgProcessor->ProcessAddFriendResult(rPDU);
            break;

        case MSG_TYPE_REFRESH_FRIEND_LIST_RESPOND:
            MsgProcessor->ProcessRefreshFriendListRespond(rPDU);
            break;

        case MSG_TYPE_DELETE_FRIEND_RESPOND:
            MsgProcessor->ProcessDeleteFriendRespond(rPDU);
            break;

        case MSG_TYPE_CHAT:
            MsgProcessor->ProcessChat(rPDU);
            break;

        case MSG_TYPE_NEW_FOLDER_RESPOND:
            MsgProcessor->ProcessNewFolderRespond(rPDU);
            break;

        case MSG_TYPE_REFRESH_FILES_LIST_RESPOND:
            MsgProcessor->ProcessRefreshFilesListRespond(rPDU);
            break;

        case MSG_TYPE_DELETE_FOLDER_RESPOND:
            MsgProcessor->ProcessDeleteFolderRespond(rPDU);
            break;

        case MSG_TYPE_DELETE_FILE_RESPOND:
            MsgProcessor->ProcessDeleteFileRespond(rPDU);
            break;

        case MSG_TYPE_RENAME_RESPOND:
            MsgProcessor->ProcessRenameRespond(rPDU);
            break;

        case MSG_TYPE_MOVE_RESPOND:
            MsgProcessor->ProcessMoveRespond(rPDU);
            break;

        case MSG_TYPE_UPLOAD_FILE_RESPOND:
            MsgProcessor->ProcessUploadRespond(rPDU);
            break;

        case MSG_TYPE_UPLOAD_FILE_ERROR:
            MsgProcessor->ProcessUploadError();
            break;

        case MSG_TYPE_DOWNLOAD_FILE_RESPOND:
            MsgProcessor->ProcessDownloadRespond(rPDU);
            break;

        case MSG_TYPE_DOWNLOAD_FILE_DATA:
            MsgProcessor->ProcessDownloadData(rPDU);
            break;

        case MSG_TYPE_DOWNLOAD_FILE_ERROR:
            MsgProcessor->ProcessDownloadError();
            break;

        case MSG_TYPE_SHARE_FILE_REQUEST:
            MsgProcessor->ProcessShareFileRequest(rPDU);
            break;

        case MSG_TYPE_SHARE_FILE_RESPOND:
            MsgProcessor->ProcessShareFileRespond(rPDU);
            break;

        default:
            break;
    }
}

void LoginPage::SendPDU(PDU *sPDU)
{
    // 读取PDU内信息，Debug输出
    qDebug() << "发送PDU";
    qDebug() << "PDU 总长度：" << sPDU->PDULen;
    qDebug() << "PDU 消息长度：" << sPDU->MsgLen;
    qDebug() << "PDU 消息类型：" << sPDU->MsgType;
    qDebug() << "PDU 参数1：" << sPDU->ParaData;
    qDebug() << "PDU 参数2：" << sPDU->ParaData + 24;
    qDebug() << "PDU 参数3：" << sPDU->ParaData + 48;
    qDebug() << "PDU 消息内容：" << sPDU->Msg;
    ClientSocket.write((char*)sPDU, sPDU->PDULen);
    delete[] sPDU;
    sPDU = NULL;
}

void LoginPage::ShowConnect()
{
    qDebug() << "连接服务器成功";
}

void LoginPage::ReceiveMsg()
{
    qDebug() << "接收到的数据长度：" << ClientSocket.bytesAvailable();
    // 读取当前接收到的所有数据
    QByteArray data = ClientSocket.readAll();
    // 将读取到的数据存入缓冲区
    buffer.append(data);

    while(buffer.size() >= sizeof(PDU))
    {
        // 将缓冲区的指针转换为PDU*类型
        PDU* rPDU = (PDU*) buffer.data();
        int MsgLen = rPDU->MsgLen;

        // 如果缓冲区大小小于消息大小，那么直接跳出循环
        if(MsgLen > buffer.size())
            break;

        // 如果有一个完整的PDU的话那就继续处理
        qDebug() << "读取PDU";
        qDebug() << "PDU 总长度：" << rPDU->PDULen;
        qDebug() << "PDU 消息长度：" << rPDU->MsgLen;
        qDebug() << "PDU 消息类型：" << rPDU->MsgType;
        qDebug() << "PDU 参数1：" << rPDU->ParaData;
        qDebug() << "PDU 参数2：" << rPDU->ParaData + 24;
        qDebug() << "PDU 参数3：" << rPDU->ParaData + 48;
        qDebug() << "PDU 消息内容：" << rPDU->Msg;

        ProcessPDU(rPDU);

        // 此时PDU已经被处理完成，缓冲区删除对应数据
        buffer.remove(0, rPDU->PDULen);
    }
}

// void LoginPage::on_Reg_ConfirmPB_clicked()
// {
//     // 进入槽函数 -> 获取输入内容 -> 内容发送给服务器 -> 服务器进入接收槽函数 -> 服务器读取PDU
//     // 读取输入框年内文字
//     QString msg = ui->Reg_UserNameLE->text();
//     if(msg.isEmpty())
//     {
//         QMessageBox::warning(this, "输入警告", "用户名不能为空");
//         return;
//     }

//     // 根据消息创建PDU
//     PDU *pdu = CreatePDU(msg.size());

//     // 设置PDU协议类型等
//     pdu->MsgType = MSG_TYPE_REGISTER_REQUEST;
//     // 是字符数组，所以不能直接赋值，不是string，需要用memcpy
//     memcpy(pdu->ParaData, "Ciallo!", 8);
//     memcpy(pdu->Msg, msg.toStdString().c_str(), msg.size());

//     // 将PDU数据写入socket，发送给服务器
//     ClientSocket.write((char*)pdu, pdu->PDULen);
//     // 删除临时用的pdu，pdu置空
//     delete[] pdu;
//     pdu = NULL;
//     qDebug() << "发送消息成功";
// }

void LoginPage::on_RegisterPB_clicked()
{
    qDebug() << "注册！";
    QString UserName = ui->UserNameLE->text();
    QString Password = ui->PasswordLE->text();

    // 将密码使用sha-256加密
    QByteArray PasswordRaw = Password.toUtf8();
    QByteArray HashedPassword = QCryptographicHash::hash(PasswordRaw, QCryptographicHash::Sha256);

    // 将哈希值转换为Base64编码，并只取前32字符
    QString HashedPasswordBase64 = HashedPassword.toBase64().left(32);

    qDebug() << "用户名：" << UserName
             << " 密码：" << HashedPasswordBase64;
    // 用户名密码判空
    if(UserName.isEmpty() && Password.isEmpty())
    {
        QMessageBox::critical(this, "用户名和密码为空", "请输入用户名和密码，不能为空！");
        return;
    }

    if(UserName.isEmpty())
    {
        QMessageBox::critical(this, "用户名为空", "请输入用户名，不能为空！");
        return;
    }

    if(Password.isEmpty())
    {
        QMessageBox::critical(this, "密码为空", "请输入密码，不能为空！");
        return;
    }

    // 用户名密码判长度
    if(UserName.toStdString().size() > 24 || Password.toStdString().size() > 32)
    {
        QMessageBox::critical(this, "用户名和密码太长", "用户名长度最长为24字符，密码为32！");
        return;
    }
    if(Password.toStdString().size() < 8)
    {
        QMessageBox::critical(this, "密码太短", "密码最短长度为8位！");
        return;
    }

    // 没有用到柔性数组，柔性数组长度为0，所以CreatePDU的参数为0
    PDU* sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_REGISTER_REQUEST;
    // toStdString将QString对象转换为std::String，方便使用C++标准库
    // c_str是C++标准库中std::string类的一个成员函数。它的作用是返回一个指向正规C字符串的指针，常用在将std::string转换为C字符串。
    memcpy(sPDU->ParaData, UserName.toStdString().c_str(), 24);
    memcpy(sPDU->ParaData + 24, HashedPasswordBase64.toStdString().c_str(), 32);
    qDebug() << "MsgType：" << sPDU->MsgType
             << " ParaData1：" << sPDU->ParaData
             << " ParaData2：" << sPDU->ParaData + 24;
    // ClientSocket.write((char*)sPDU, sPDU->PDULen);
    // delete[] sPDU;
    // sPDU = NULL;
    SendPDU(sPDU);
    qDebug() << "发送注册信息成功";
}


void LoginPage::on_LoginPB_clicked()
{
    qDebug() << "登录！";
    // QString UserName = ui->UserNameLE->text();
    CurrentUser = ui->UserNameLE->text();
    QString Password = ui->PasswordLE->text();

    // 将密码使用sha-256加密
    QByteArray PasswordRaw = Password.toUtf8();
    QByteArray HashedPassword = QCryptographicHash::hash(PasswordRaw, QCryptographicHash::Sha256);

    // 将哈希值转换为Base64编码，并只取前32字符
    QString HashedPasswordBase64 = HashedPassword.toBase64().left(32);

    qDebug() << "用户名：" << CurrentUser
             << " 密码：" << HashedPasswordBase64;

    // 用户名密码判空
    if(CurrentUser.isEmpty() && Password.isEmpty())
    {
        QMessageBox::critical(this, "用户名和密码为空", "请输入用户名和密码，不能为空！");
        return;
    }

    if(CurrentUser.isEmpty())
    {
        QMessageBox::critical(this, "用户名为空", "请输入用户名，不能为空！");
        return;
    }

    if(Password.isEmpty())
    {
        QMessageBox::critical(this, "密码为空", "请输入密码，不能为空！");
        return;
    }

    // 用户名密码判长度
    if(CurrentUser.toStdString().size() > 24 || Password.toStdString().size() > 32)
    {
        QMessageBox::critical(this, "用户名和密码太长", "用户名长度最长为24字符，密码为32！");
        return;
    }
    if(Password.toStdString().size() < 8)
    {
        QMessageBox::critical(this, "密码太短", "密码最短长度为8位！");
        return;
    }

    // 没有用到柔性数组，柔性数组长度为0，所以CreatePDU的参数为0
    PDU* sPDU = CreatePDU(0);
    sPDU->MsgType = MSG_TYPE_LOGIN_REQUEST;
    // toStdString将QString对象转换为std::String，方便使用C++标准库
    // c_str是C++标准库中std::string类的一个成员函数。它的作用是返回一个指向正规C字符串的指针，常用在将std::string转换为C字符串。
    memcpy(sPDU->ParaData, CurrentUser.toStdString().c_str(), 24);
    memcpy(sPDU->ParaData + 24, HashedPasswordBase64.toStdString().c_str(), 32);
    SendPDU(sPDU);
    qDebug() << "发送登录信息成功";
}
