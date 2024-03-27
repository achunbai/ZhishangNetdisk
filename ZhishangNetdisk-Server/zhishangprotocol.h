#ifndef ZHISHANGPROTOCOL_H
#define ZHISHANGPROTOCOL_H

#include <cstdlib>
#include <string>
#include <QStringList>

/*
 * 服务器在和客户端进行通信的时候可能会遇到幻包和脏包的情况
 *  客户端和服务器之间发送的数据可能不能只由一个数据包完整发送
 *  如果内容过长，会对其进行拆分再发送
 *  服务器和客户端需要知道对方发送的消息是否完整
 *  如果一方在发送多条消息间隔时间过短，可能会造成另一方将多条消息读成一条消息的情况
 *
 * 半包（Half-Packet）：
 *  半包是指一个完整的数据包被TCP拆分成两个或多个数据包进行发送。
 *  这种情况通常是由于数据包的大小超过了TCP的最大传输单元（MTU）导致的。
 *  接收方在接收到数据时，需要将这些被拆分的数据包重新组合成一个完整的数据包。
 *
 * 粘包（Stick-Packet）：
 *  粘包是指多个数据包被TCP合并为一个数据包进行发送。
 *  这种情况通常是由于网络的延迟或者数据包的大小小于TCP的最小传输单元导致的。
 *  接收方在接收到数据时，需要将这个大的数据包拆分成多个原始的数据包。
 *
 * 避免半包和粘包的方法：
 *  1. 标注数据长度（实际数据长度，协议总长度）
 *  2. 隔开数据内容（柔性数据存大数据，先申请64位空间存储小数据，注意柔性数组放在最后）
 *  3. 标注数据类型
 *
 *  可以定义一个PDU结构体来存储需要传输的数据
 *  数据内容可以使用柔性数组来定义，柔性数组在声明时不占用空间，要存多少东西就需要额外申请多大的空间
 *
 */

typedef unsigned int uint;

// 用枚举来设置消息类型，方便查看和记忆
// 注意枚举中是逗号分隔
enum MSG_TYPE
{
    MSG_TYPE_MIN = 0,
    // 注册
    MSG_TYPE_REGISTER_REQUEST,
    MSG_TYPE_REGISTER_RESPOND,
    // 登录
    MSG_TYPE_LOGIN_REQUEST,
    MSG_TYPE_LOGIN_RESPOND,
    // 查找用户
    MSG_TYPE_SEARCH_USER_REQUEST,
    MSG_TYPE_SEARCH_USER_RESPOND,
    // 在线用户
    MSG_TYPE_SHOW_ONLINE_USERS_REQUEST,
    MSG_TYPE_SHOW_ONLINE_USERS_RESPOND,
    // 好友请求
    MSG_TYPE_ADD_FRIEND_REQUEST,
    MSG_TYPE_ADD_FRIEND_RESPOND,
    MSG_TYPE_ADD_FRIEND_RESULT,
    // 刷新好友列表
    MSG_TYPE_REFRESH_FRIEND_LIST_REQUEST,
    MSG_TYPE_REFRESH_FRIEND_LIST_RESPOND,
    // 删除好友
    MSG_TYPE_DELETE_FRIEND_REQUEST,
    MSG_TYPE_DELETE_FRIEND_RESPOND,
    // 聊天
    MSG_TYPE_CHAT,
    // 创建文件夹
    MSG_TYPE_NEW_FOLDER_REQUEST,
    MSG_TYPE_NEW_FOLDER_RESPOND,
    // 刷新文件列表
    MSG_TYPE_REFRESH_FILES_LIST_REQUEST,
    MSG_TYPE_REFRESH_FILES_LIST_RESPOND,
    // 删除文件夹
    MSG_TYPE_DELETE_FOLDER_REQUEST,
    MSG_TYPE_DELETE_FOLDER_RESPOND,
    // 删除文件
    MSG_TYPE_DELETE_FILE_REQUEST,
    MSG_TYPE_DELETE_FILE_RESPOND,
    // 重命名
    MSG_TYPE_RENAME_REQUEST,
    MSG_TYPE_RENAME_RESPOND,
    // 移动文件
    MSG_TYPE_MOVE_REQUEST,
    MSG_TYPE_MOVE_RESPOND,
    // 上传文件
    MSG_TYPE_UPLOAD_FILE_REQUEST,
    MSG_TYPE_UPLOAD_FILE_RESPOND,
    MSG_TYPE_UPLOAD_FILE_DATA,
    MSG_TYPE_UPLOAD_FILE_SUCCESS,
    MSG_TYPE_UPLOAD_FILE_ERROR,
    // 下载文件
    MSG_TYPE_DOWNLOAD_FILE_REQUEST,
    MSG_TYPE_DOWNLOAD_FILE_RESPOND,
    MSG_TYPE_DOWNLOAD_FILE_READY,
    MSG_TYPE_DOWNLOAD_FILE_DATA,
    MSG_TYPE_DOWNLOAD_FILE_ERROR,
    // 分享文件
    MSG_TYPE_SHARE_FILE_REQUEST,
    MSG_TYPE_SHARE_FILE_RESPOND,


    MSG_TYPE_MAX = 0x00ffffff
};

// 枚举文件类型
enum FILE_TYPE
{
    FILE_TYPE_FOLDER = 0,
    FILE_TYPE_FILE,
    FILE_TYPE_VIDEO,
    FILE_TYPE_AUDIO,
    FILE_TYPE_IMAGE,
    FILE_TYPE_DOCUMENT,
    FILE_TYPE_TEXT,
    FILE_TYPE_OTHER
};

// 定义协议数据单元PDU
typedef struct ProtocolDataUnit{
    // 协议总长度
    // 总长度 = 实际消息长度 + PDU的长度
    uint PDULen;
    // 实际消息长度
    uint MsgLen;
    // 消息类型
    uint MsgType;
    // 参数
    char ParaData[64];
    // 实际消息，柔性数组
    char Msg[];

}PDU;

// 定义文件类型
typedef struct FileInfo{
    // 文件类型
    int FileType;
    // 文件名
    char FileName[64];
}FI;

// 定义创建PDU的函数，注意头文件中仅声明
PDU* CreatePDU(uint MsgLen);
// 返回文件后缀名类型
int GetFileType(const QString &suffix);

#endif // ZHISHANGPROTOCOL_H
