#include "header.h"


Header::Header(size_t id,
               unsigned long construction_time) :
        _id(id),
        _construction_time(construction_time)
{ }

size_t Header::id() const
{
    return _id;
}

unsigned long Header::construction_time() const
{
    return _construction_time;
}
