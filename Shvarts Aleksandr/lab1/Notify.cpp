#include "Notify.h"
#include <sys/select.h>

Notify::Notify() {
#ifdef IN_NONBLOCK
    fd = inotify_init1(IN_NONBLOCK);
#else
    fd = inotify_init();
#endif
    if (fd <= 0) {
        syslog(LOG_ERR, "Error with creation Notify");
        exit(EXIT_FAILURE);
    }
    FD_ZERO(&watch_set);
    FD_SET(fd, &watch_set);
    syslog(LOG_INFO, "Notify created %d", fd);
}

Notify::~Notify() {
    if (this->fd)
        close(this->fd);
}

void sigterm(int signo) {
    syslog(LOG_CRIT, "%d\n", signo);
}

void Notify::writeChanges() {
    char buffer[EVENT_BUF_LEN];

    struct timeval tval = {2, 0};

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGTERM);
    sigaddset(&signals, SIGHUP);
    sigprocmask(SIG_BLOCK, &signals, nullptr);

    while (select(fd + 1, &watch_set, nullptr, nullptr, &tval) >= 0) {
        sigpending(&signals);
        if (sigismember(&signals, SIGTERM)) {
            /* got a SIGINT */
//            sigpause(SIGTERM);
            syslog(LOG_NOTICE, "I am returning home!");
            exit(EXIT_SUCCESS);
        } else if (sigismember(&signals, SIGHUP)) {
            sigset_t newsig;
            sigemptyset(&newsig);
            sigsuspend(&newsig);
            return;
        }
        // handling config reloading
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            continue;
        }
        struct inotify_event* event;
        for (int i = 0; i < length; i += EVENT_SIZE + event->len) {
            event = (struct inotify_event*) &buffer[i];
            if (event->len) {
                if (event->mask & IN_ISDIR) { // For directories
                    if (event->mask & IN_CREATE) {
                        syslog(LOG_INFO, "New directory %s was created.\n", event->name);
                    } else if (event->mask & IN_ATTRIB) {
                        syslog(LOG_INFO, "Directory %s metadata changed.\n", event->name);
                    } else if (event->mask & IN_OPEN) {
                        syslog(LOG_INFO, "Directory %s was opened.\n", event->name);
                    } else if (event->mask & IN_DELETE) {
                        syslog(LOG_INFO, "File %s was deleted.\n", event->name);
                    }
                } else { // For files
                    if (event->mask & IN_CREATE) {
                        syslog(LOG_INFO, "New file %s was created.\n", event->name);
                    }
                    if (event->mask & IN_ACCESS) {
                        syslog(LOG_INFO, "File %s was accessed.\n", event->name);
                    }
                    if (event->mask & IN_ATTRIB) {
                        syslog(LOG_INFO, "File %s metadata changed.\n", event->name);
                    }
                    if (event->mask & IN_MODIFY) {
                        syslog(LOG_INFO, "File %s was modified.\n", event->name);
                    }
                    if (event->mask & IN_OPEN) {
                        syslog(LOG_INFO, "File %s was opened.\n", event->name);
                    }
                    if (event->mask & IN_DELETE) {
                        syslog(LOG_INFO, "File %s was deleted.\n", event->name);
                    }
                }
            }

        }
    }
    if (errno == EINTR)
        syslog(LOG_INFO, "Interrupted by SIGTERM.");
    else
        syslog(LOG_ERR, "pselect()");
    //        syslog(LOG_ERR, "%d\n", errno);
    //        return;
}

void Notify::loadConfig(const std::string& path) {
    files.clear();
    std::fstream file(path);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of("\t\n\v\f\r ")); // left trim
            line.erase(line.find_last_not_of("\t\n\v\f\r ") + 1); // right trim
            if (line.empty()) {
                continue;
            }
            fs::path checkedFile(line);
            if (fs::exists(checkedFile) && fs::is_directory(checkedFile)) {
                syslog(LOG_INFO, "Add directory %s", checkedFile.c_str());
                files.insert(checkedFile.c_str());
                for (const auto& curFile: fs::recursive_directory_iterator(checkedFile)) {
                    if (curFile.is_directory()) {
                        std::stringstream s;
                        s << curFile;
                        files.insert(s.str());
                        syslog(LOG_INFO, "Add directory %s", s.str().c_str());
                    }
                }
            } else {
                syslog(LOG_ERR, "Incorrect config, directory does not exist.");
                syslog(LOG_EMERG, "%s\n", checkedFile.c_str());
                exit(EXIT_FAILURE);
            }
        }
        for (const auto& curFile: files) {
            int wd = inotify_add_watch(fd, curFile.c_str(), IN_ALL_EVENTS);
            if (wd < 0) {
                syslog(LOG_WARNING, "Can not add to watch list %s", curFile.c_str());
            } else {
                syslog(LOG_INFO, "Add to watch list %s", curFile.c_str());
            }

        }
        file.close();
    } else {
        syslog(LOG_ERR, "Error with opening config");
        exit(EXIT_FAILURE);
    }
}
