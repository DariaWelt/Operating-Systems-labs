#include <QApplication>
#include <unistd.h>
#include "string.h"
#include "gui.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"

GUI::MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
    timer->start(10);
}

void GUI::MainWindow::SetGUI(GUI* gui) {
    this->gui = gui;
    std::string windowName = gui->windowName + "[" + std::to_string(getpid()) + "]";
    setWindowTitle(windowName.c_str());
}

void GUI::MainWindow::send() {
    Message msg;
    std::string message = gui->windowName + ": " + ui->lineEdit->text().toLocal8Bit().data();
    size_t size = message.size() > MSG_MAX_SIZE ? MSG_MAX_SIZE - 1 : message.size();
    strncpy(msg.text, message.c_str(), size);
    gui->guiWriteCallback(msg);
    ui->lineEdit->clear();
    ui->listWidget->addItem(message.c_str());
}

void GUI::MainWindow::tick() {
    if (!gui->guiIsRunningCallback()) {
        this->close();
    }

    if (gui->isConnected) {
        ui->statusbar->showMessage("Connected");
    } else {
        ui->statusbar->showMessage("Not connected");
    }

    Message msg = {0};
    while (gui->guiReadCallback(&msg)) {
        ui->listWidget->addItem(msg.text);
    }
}

GUI::MainWindow::~MainWindow() {
    delete ui;
}

int GUI::Run() {
    int argc = 1;
    char* args[] = { (char*)windowName.c_str() };
    QApplication app(argc,args);
    MainWindow window;
    window.SetGUI(this);
    window.show();
    return app.exec();
}
