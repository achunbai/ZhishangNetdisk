# 记得要添加对应的模块，目前需要使用网络，故需要添加网络模块
QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chat.cpp \
    downloadprogress.cpp \
    files.cpp \
    friends.cpp \
    loginpage.cpp \
    main.cpp \
    mainwindow.cpp \
    msghandler.cpp \
    onlineusers.cpp \
    share.cpp \
    uploadprogress.cpp \
    zhishangprotocol.cpp

HEADERS += \
    chat.h \
    downloadprogress.h \
    files.h \
    friends.h \
    loginpage.h \
    mainwindow.h \
    msghandler.h \
    onlineusers.h \
    share.h \
    uploadprogress.h \
    zhishangprotocol.h

FORMS += \
    chat.ui \
    downloadprogress.ui \
    files.ui \
    friends.ui \
    loginpage.ui \
    mainwindow.ui \
    onlineusers.ui \
    share.ui \
    uploadprogress.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    client_config.qrc
