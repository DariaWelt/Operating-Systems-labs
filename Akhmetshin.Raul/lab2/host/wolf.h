#pragma once
#include <QApplication>
#include <QWidget>
#include <QTime>
#include "mainwindow.h"
#include <iostream>
#include <conn.h>

#include <signal.h>

#include <vector>
#include <memory>
#include <map>


class Wolf {
private:
    using pid_thread_t = std::map<pid_t, pthread_t>;
    using wolf_ptr = std::unique_ptr<Wolf>;

private:
    static wolf_ptr wolf;

    size_t max_goats_num;
    bool continue_the_game = true;

    size_t cur_turns_counter_without_alive_goats = 0;

    size_t goats_counter;
    pid_thread_t goats;
    std::vector<Connection*> connections;

    size_t wolf_number;

public:
    Wolf();
    ~Wolf();

    static Wolf& instanse();

    int run();
    void set_max_goats_num(size_t goats_num);

    QApplication *q_app;
    MainWindow *wnd;


private:
    static void signal_handler(int signal, siginfo_t * info, void * context);
    static void* game(void* argv);

    // static bool chase_goat(bool is_alive, size_t goat_num);

    size_t generate_number() const;
    bool chase_goat(bool is_alive, size_t goat_num) const;

    void define_game_status(bool is_all_goats_dead);

    void next_iter();
};
