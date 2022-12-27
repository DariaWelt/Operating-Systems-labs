#ifndef LAB2_CONN_H
#define LAB2_CONN_H

#include "../settings/GameSettings.h"

class Conn {
public:

    static Conn* createConnection();

    virtual bool open(size_t hostPid, bool isCreator) = 0;

    virtual bool read(void* buf, size_t count) = 0;

    virtual bool write(void* buf, size_t count) = 0;

    virtual bool close() = 0;

    virtual ~Conn() = default;

    enum class MSG_TYPE
    {
        TO_WOLF = 0,
        TO_GOAT
    };

    struct Msg {
        MSG_TYPE type;
        int data;
    };

};

#endif //LAB2_CONN_H
