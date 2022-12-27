#ifndef __GUI_H
#define __GUI_H
#include <atomic>
#include<iostream>
#include "../conn/IConn.hpp"

enum{
  MAX_MESSAGE_LEN = 400
};

using writer = void(*)(IConn::Message m);
using reader = bool(*)(IConn::Message *m);
using status_callback = bool(*)(void);

class GUI
{
  private:
    class ChatWindow;

    writer _send;
    reader _get;
    status_callback _is_running;
    std::string _name;

    GUI() = delete;
  public:
    GUI(std::string name, writer writer, reader reader, status_callback isRun) :
      _send(writer), _get(reader), _is_running(isRun), _name(name) {};
    std::string buildMessageGui();
    int start(void);
    ~GUI();
};

#endif //__GUI_H