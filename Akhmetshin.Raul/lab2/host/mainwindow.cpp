#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::table_init(int a){

    ui->table->setColumnCount(3); // Указываем число колонок
    ui->table->setShowGrid(true);
    QStringList headers;
    headers.append("pid");
    headers.append("num");
    headers.append("status");
    ui->table->setHorizontalHeaderLabels(headers);
    ui->table->setEditTriggers(QTableWidget::NoEditTriggers);

    for (int i=0; i<a; i++) {
        ui->table->insertRow(0);
        ui->table->setItem(0,0, new QTableWidgetItem(QString::number(0)));
    }
    max_count=a;

}

void MainWindow::pid_fil(int a) {

    ui->table->setItem(pid_cnt, 0, new QTableWidgetItem(QString::number(a)));
    pid_cnt++;

}

static int _random(int minNum, int maxNum) {
    return int((1.0 * rand() + 1) / (1.0 * RAND_MAX + 1) * (maxNum - minNum) + minNum);
}

int MainWindow::GetNumber() {

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &MainWindow::on_button_clicked, &loop, &QEventLoop::quit );
    connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
    ui->button->setEnabled(true);
    timer.start(timeout);
    loop.exec();
    int num;
    if(timer.isActive()) {
        num = ui->wolfBox->value();
    }
    else {
        num = _random(0, 101);
    }
    ui->button->setEnabled(false);
    ui->wolfBox->setValue(num);
    return num;
}

void MainWindow::setStatus(int pid, int num, bool stat) {

    for (int i=0; i<max_count; i++){
        QTableWidgetItem *item = ui->table->item(i,0);
        if (item->text().toStdString() == std::to_string(pid) ){
            ui->table->setItem(i,1, new QTableWidgetItem(QString::number(num)));
            ui->table->setItem(i,2, new QTableWidgetItem(stat? "Alive": "Dead"));
            break;
        }
    }

}

void MainWindow::setWolfPid(int pid) {
    ui->pid_label->setText(QString::number(pid));
}

void MainWindow::setGameStat(std::string stat) {
    ui->status->setText(QString::fromStdString(stat));
}

void MainWindow::setWolfNumber(int num) {
    ui->wolf_num->setText(QString::number(num));
}

void MainWindow::setAliveCount(int num) {
    ui->alive_count->setText(QString::number(num));
    ui->dead_count->setText(QString::number(max_count-num));
}
