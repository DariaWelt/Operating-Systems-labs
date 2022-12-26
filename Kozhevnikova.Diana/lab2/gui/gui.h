#pragma once

#include <QMainWindow>
#include <QtGui>
#include <QPushButton>

#include <string>

#include "../queueMsg.h"

using read_callback_t = void(*)(Message msg);
using write_callback_t = bool(*)(Message *msg);
using controll_callback_t = bool(*)();

namespace Ui {
  class ChatWin;
}

class ChatWin : public QMainWindow {
    Q_OBJECT
private:
    Ui::ChatWin *ui;
    std::string windowName;
    read_callback_t winReadCallback;
    write_callback_t winWriteCallback;
    controll_callback_t winControlCallback;

    QTimer *timer;
    QPushButton *button;

    void initTimer();
    std::string constructMessage();
    
private slots:
    void sendMessage();
    void updateTimer();

public:
    ChatWin(std::string name, 
        read_callback_t readCallback, 
        write_callback_t writeCallback, 
        controll_callback_t controllCallback);
    ~ChatWin();
};