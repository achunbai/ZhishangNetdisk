#include "zhishangprotocol.h"

// 定义PDU创建函数
PDU *CreatePDU(uint MsgLen)
{
    // PDU长度
    // PDU长度 = 其他部分长度（定义的时候Msg为柔性数组，不占空间，所以可以用sizeof求出来） + 消息长度
    uint PDULen = sizeof(PDU) + MsgLen;
    // // malloc用free进行释放
    // PDU* tPDU = std::malloc(PDULen);
    // if(tPDU == NULL)
    // {
    //     exit(1);
    // }
    // memset(tPDU, 0, PDULen);

    // 使用new来动态分配一个PDU，需要用delete[]进行释放
    PDU* tPDU = (PDU*) new char[PDULen]();
    // 错误处理
    if(tPDU == NULL)
    {
        exit(1);
    }

    // 设置PDU总长度和消息长度
    tPDU->PDULen = PDULen;
    tPDU->MsgLen = MsgLen;

    return tPDU;
}

int GetFileType(const QString &suffix)
{
    static QStringList DocumentSuffixes = {"doc", "docx", "pdf", "xls", "xlsx", "ppt", "pptx", "wps", "et", "dps", "odt", "ods", "odp", "rtf"};
    static QStringList TextSuffixes = {"txt", "md", "log"};
    static QStringList AudioSuffixes = {"mp3", "wav", "flac", "aac", "ogg", "wma"};
    static QStringList VideoSuffixes = {"mp4", "avi", "mkv", "flv", "mov", "wmv"};
    static QStringList ImageSuffixes = {"jpg", "png", "gif", "bmp", "jpeg", "svg", "ico"};

    if(DocumentSuffixes.contains(suffix))
    {
        return FILE_TYPE_DOCUMENT;
    }
    else if(AudioSuffixes.contains(suffix))
    {
        return FILE_TYPE_AUDIO;
    }
    else if(VideoSuffixes.contains(suffix))
    {
        return FILE_TYPE_VIDEO;
    }
    else if(ImageSuffixes.contains(suffix))
    {
        return FILE_TYPE_IMAGE;
    }
    else if(TextSuffixes.contains(suffix))
    {
        return FILE_TYPE_TEXT;
    }

    return FILE_TYPE_FILE;
}
