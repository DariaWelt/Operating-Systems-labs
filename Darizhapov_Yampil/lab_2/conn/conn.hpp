#pragma once
#include "message.hpp"
#include <cstdlib>

class Conn
{
public:
    virtual void read(Message &msg) = 0;
    virtual void write(const Message &msg) = 0;
    static Conn* create(bool isHost);
    virtual ~Conn() = default;
};
