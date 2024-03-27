QT       += core gui network sql
# 记得添加network模块
# 需要对数据库进行操作，所以也需要添加sql模块

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO

QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO

QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

SOURCES += \
    databaseoperate.cpp \
    main.cpp \
    msghandler.cpp \
    server_main.cpp \
    zhishangprotocol.cpp \
    zhishangtcpserver.cpp \
    zhishangtcpsocket.cpp

HEADERS += \
    databaseoperate.h \
    msghandler.h \
    server_main.h \
    zhishangprotocol.h \
    zhishangtcpserver.h \
    zhishangtcpsocket.h

FORMS += \
    server_main.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Server_Config.qrc
