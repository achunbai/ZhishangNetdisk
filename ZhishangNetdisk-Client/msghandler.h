#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include "zhishangprotocol.h"

class MsgHandler
{
public:
    MsgHandler();
    void ProcessRegisterRespond(const PDU *rPDU);
    void ProcessLoginRespond(const PDU *rPDU);
    void ProcessSearchUserRespond(const PDU *rPDU);
    void ProcessShowOnlieneUserRespond(const PDU *rPDU);
    void ProcessAddFriendRequest(const PDU *rPDU);
    void ProcessAddFriendRespond(const PDU *rPDU);
    void ProcessAddFriendResult(const PDU *rPDU);
    void ProcessRefreshFriendListRespond(const PDU *rPDU);
    void ProcessDeleteFriendRespond(const PDU *rPDU);
    void ProcessChat(const PDU *rPDU);
    void ProcessNewFolderRespond(const PDU *rPDU);
    void ProcessRefreshFilesListRespond(const PDU *rPDU);
    void ProcessDeleteFolderRespond(const PDU *rPDU);
    void ProcessDeleteFileRespond(const PDU *rPDU);
    void ProcessRenameRespond(const PDU *rPDU);
    void ProcessMoveRespond(const PDU *rPDU);
    void ProcessUploadRespond(const PDU *rPDU);
    void ProcessUploadSuccess();
    void ProcessUploadError();
    void ProcessDownloadRespond(const PDU *rPDU);
    void ProcessDownloadData(const PDU *rPDU);
    void ProcessDownloadError();
    void ProcessShareFileRequest(const PDU *rPDU);
    void ProcessShareFileRespond(const PDU *rPDU);
};

#endif // MSGHANDLER_H
