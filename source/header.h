#pragma once


#include <cstdint>


using std::size_t;


class Header
{
public:
    Header(size_t id,
           unsigned long construction_time);

    size_t id() const;
    unsigned long construction_time() const;


private:
    size_t _id;
    unsigned long _construction_time;
};