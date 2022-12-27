#ifndef FUTUREWEATHER_H
#define FUTUREWEATHER_H
#include "../connection/message.h"
#include "../connection/connection.h"
#include <semaphore.h>

class futureWeather
{
public:
    static futureWeather *getInstance(int id);
    void start();
    bool openConnection();
    void setConnection(connection _connection);
    
private:
    connection _connection;
    sem_t *_semaphoreHost;
    sem_t *_semaphoreClient;
    std::string _semClientName;
    std::string _semHostName;

    int _clientId;

    futureWeather(int id);
    futureWeather(futureWeather &weather);
    futureWeather &operator=(futureWeather &weather);
    void terminate(int signum);
    int getWeather();
    static void signalHandler(int signum);
};
#endif
