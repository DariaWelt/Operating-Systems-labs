#include <QMainWindow>
#include <QtGui>
#include <QPushButton>
#include "gui.h"

namespace Ui {
  class MainWindow;
}

class GUI;

class GUI::MainWindow : public QMainWindow
{
    Q_OBJECT
  private:
    QTimer* timer;
    QPushButton* button;
    GUI* gui;
    Ui::MainWindow* ui;
  private slots:
    void tick();
    void send();
  public:
    explicit MainWindow(QWidget* parent = 0);

    void SetGUI(GUI *gui);
    ~MainWindow();
};
