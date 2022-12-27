#ifndef LAB2_CLIENT_INFO_H
#define LAB2_CLIENT_INFO_H
#include "IConn.hpp"

class  ConnInfo{
public:
    IConn *conn;
    sem_t *_sem_client;
    sem_t *_sem_host;
};

#endif //LAB2_CLIENT_INFO_H