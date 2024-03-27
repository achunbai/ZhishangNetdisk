#include "server_main.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server_Main::GetInstance().show();
    return a.exec();
}
