#include "header.h"


Header::Header(MessageType type,
               size_t id,
               unsigned long construction_time) :
        _type(type),
        _id(id),
        _construction_time(construction_time)
{ }

MessageType Header::type() const
{
    return _type;
}

size_t Header::id() const
{
    return _id;
}

unsigned long Header::construction_time() const
{
    return _construction_time;
}