#include <QMainWindow>
#include <QtGui>
#include <QPushButton>
#include "gui.h"

namespace Ui {
  class ChatWindow;
}

class GUI;

class GUI::ChatWindow : public QMainWindow
{
    Q_OBJECT
  public:
    explicit ChatWindow(QWidget *parent = 0);
    void SetGUI(GUI *gui);
    
    ~ChatWindow();

  private slots:
    void update();
    void send();
  private:
    std::string buildMessageGui();
    QTimer *_timer;
    QPushButton *_button;
    GUI *_gui;
    Ui::ChatWindow *_ui;
};