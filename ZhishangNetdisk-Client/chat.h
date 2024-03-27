#ifndef CHAT_H
#define CHAT_H

#include <QWidget>

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT

public:
    explicit Chat(QWidget *parent = nullptr);
    ~Chat();
    QString& GetTargetUserName();
    void SetTitle(const QString &UserName);
    void SetMsg(const char *UserName, const char *Message);

private slots:
    void on_SendPB_clicked();

private:
    Ui::Chat *ui;
    QString TargetUserName;
};

#endif // CHAT_H
