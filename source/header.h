#pragma once


#include <cstdint>


using std::size_t;


class Header
{
public:
    Header(size_t id);

    size_t id() const;


private:
    size_t _id;
};