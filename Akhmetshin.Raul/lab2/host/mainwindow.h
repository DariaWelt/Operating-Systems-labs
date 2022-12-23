#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void table_init(int a);
    void pid_fil(int a);
    int max_count;
    int pid_cnt=0;
    int GetNumber();
    void setStatus(int pid, int num, bool stat);
    void setWolfPid(int pid);
    void setGameStat(std::string stat);
    void setWolfNumber(int num);
    void setAliveCount(int num);

signals:
    void on_button_clicked();

private:
    Ui::MainWindow *ui;
    const int timeout = 3000;
};
#endif // MAINWINDOW_H
