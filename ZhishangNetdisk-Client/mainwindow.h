#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "friends.h"
#include "files.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    static MainWindow& GetInstance();
    Files *GetFilesPage();
    Friends *GetFriendsPage();
    void SetTitle(const QString &UserName);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    explicit MainWindow(QWidget *parent = nullptr);
    // 删除赋值运算符
    MainWindow& operator=(MainWindow&) = delete;
    // 删除拷贝构造
    MainWindow(const MainWindow& instance) = delete;
};

#endif // MAINWINDOW_H
