#pragma once


#include <cstdint>

#include "message-type.h"


using std::size_t;


class Header
{
public:
    Header(MessageType type,
           size_t id,
           unsigned long construction_time);

    MessageType type() const;
    size_t id() const;
    unsigned long construction_time() const;


private:
    MessageType _type;
    size_t _id;
    unsigned long _construction_time;
};