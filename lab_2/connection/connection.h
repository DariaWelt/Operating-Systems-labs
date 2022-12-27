#ifndef CONNECTION_H
#define CONNECTION_H
#include <stdlib.h>
#include <string>

class connection
{
public:
    bool open(size_t id, bool create);
    bool close();

    bool read(void *buf, size_t count);
    bool write(void *buf, size_t count);

    connection() = default;
    ~connection() = default;

private:
    std::string _connName;
    bool _isCreated;
    int *_fileDescr;
};
#endif