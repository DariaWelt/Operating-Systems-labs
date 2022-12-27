#include <QApplication>
#include <unistd.h>
#include "string.h"
#include "gui.h"
#include "ChatWindow.h"
#include "ui_ChatWindow.h"
#include <syslog.h>

GUI::ChatWindow::ChatWindow(QWidget *parent) : QMainWindow(parent), _ui(new Ui::ChatWindow)
{
  _ui->setupUi(this);
  _timer = new QTimer(this);
  QObject::connect(_timer, SIGNAL(timeout()), this, SLOT(update()));
  _timer->start(8);
}

void GUI::ChatWindow::SetGUI(GUI *gui)
{
  _gui = gui;
  std::string windowName = _gui->_name + " " + std::to_string(getpid());
  setWindowTitle(windowName.c_str());
}

std::string GUI::ChatWindow::buildMessageGui(){
    std::string out_msg;
    out_msg += _gui->_name;
    out_msg += " >> ";
    out_msg += _ui->inputWidget->text().toLocal8Bit().data();
    return out_msg;
}

void GUI::ChatWindow::send() {
  IConn::Message msg;
  std::string out_msg = buildMessageGui();
  strncpy(msg.msg, out_msg.c_str(), std::max((int)out_msg.size(), (int)MAX_MESSAGE_LEN));
  _gui->_send(msg);
  _ui->inputWidget->clear();
  _ui->chatWidget->addItem(out_msg.c_str());
}

void GUI::ChatWindow::update() {
  IConn::Message msg;
  while (_gui->_get(&msg))
    _ui->chatWidget->addItem(msg.msg);
}

GUI::ChatWindow::~ChatWindow() {
  delete _timer;
  delete _ui;
}

int GUI::start() {
  int argc = 1;
  char* args[] = { (char*)_name.c_str() };
  syslog(LOG_INFO, "[INFO] chat %s", (*args));
  QApplication app(argc,args);
  ChatWindow window;
  window.SetGUI(this);
  window.show();
  return app.exec();
}

GUI::~GUI() {}
