#pragma once

#include "conn.h"

#include <string>
#include <unistd.h>
#include <unordered_map>

struct PipePair
{
    struct PipeDescriptors
    {
        int read;
        int write;
    };
    PipeDescriptors hostDesriptors;
    PipeDescriptors clientDesriptors;
};

class Pipe : public Connection
{
private:
    std::string name;
    bool isHost = false;
    PipePair pipePair;
    int readDescriptor = -1;
    int writeDescriptor = -1;

public:
    static std::unordered_map<std::string, PipePair> pipeDict;
    Pipe() = delete;
    Pipe(const std::string &name, bool isHost, const PipePair &pipePair);
    bool open();
    bool read(Message &msg);
    bool write(const Message &msg);
    bool close();
};