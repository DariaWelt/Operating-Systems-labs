#ifndef __GUI_H_
#define __GUI_H_

#include <atomic>
#include <string>
#include "../messageQueue/messageQueue.h"

using write_callback = void(*)(Message msg);
using read_callback = bool(*)(Message* msg);
using is_running_callback = bool(*)(void);

class GUI
{
  private:
    std::string windowName;

    class MainWindow;

    read_callback guiReadCallback;
    write_callback guiWriteCallback;
    is_running_callback guiIsRunningCallback;

    // Flag to check chat status
    std::atomic<bool> isConnected = false;
  public:
    GUI(std::string name, read_callback readCallback, write_callback writeCallback, is_running_callback isRunningCallback)
        : windowName(name), guiReadCallback(readCallback), guiWriteCallback(writeCallback), guiIsRunningCallback(isRunningCallback) {};

    void SetConnected(bool isConnected) { this->isConnected = isConnected; };

    int Run(void);
    
    ~GUI(void) = default;
};
#endif
