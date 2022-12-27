#ifndef MESSAGE_H
#define MESSAGE_H

const int timeout = 5;

class message
{
public:
    message() = default;
    message(int d, int m, int y) : _temp(0), _day(d), _month(m), _year(y) {};
    message(double t, int d, int m, int y) : _temp(t), _day(d), _month(m), _year(y) {};
    void setDay(int day) { _day = day; }
    void setMonth(int m) { _month = m; }
    void setYear(int y) { _year = y; }
    void setTemp(int t) { _temp = t; }
    int getDay() { return _day; }
    int getMonth() { return _month; }
    int getYear() { return _year; }
    int getTemp() { return _temp; }

private:
    int _temp;
    int _day;
    int _month;
    int _year;
};
#endif