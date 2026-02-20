#pragma once

// Minimal stub for DateTime for native tests
class DateTime {
    int _y, _m, _d, _hh, _mm, _ss;
public:
    DateTime(int y, int m, int d, int hh = 0, int mm = 0, int ss = 0)
        : _y(y), _m(m), _d(d), _hh(hh), _mm(mm), _ss(ss) {}
    int unixtime() const { return 1708420800; } // fixed timestamp for test
    int hour() const { return _hh; }
};

// Minimal Serial stub
struct Serial_t {
    void println(const char*) {}
};
static Serial_t Serial;
