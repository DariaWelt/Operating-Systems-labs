
#include <iostream>
#include <unistd.h>
#include <QWidget>
#include "wolf.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Invalid arguments. Must be one args - max clients num" << std::endl;
        return -1;
    }

    int goats_num = std::stoi(argv[1]);

    if (goats_num <= 0) {
        std::cout << "Goats num should be positive integer" << std::endl;
        return -1;
    }


    std::cout << getpid() << std::endl;

    Wolf& wolf = Wolf::instanse();

    wolf.q_app = new QApplication(argc,argv);
    wolf.wnd = new MainWindow;
    wolf.set_max_goats_num(goats_num);
    wolf.wnd->show();

    wolf.wnd->table_init(int(goats_num));

    wolf.wnd->setWolfPid(getpid());
    wolf.wnd->setGameStat("Waiting for connection...");
    wolf.wnd->update();
    //wolf.run();

    return wolf.q_app->exec();
}