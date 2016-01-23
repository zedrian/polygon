#include "header.h"


Header::Header(size_t id) :
_id(id)
{ }

size_t Header::id() const
{
    return _id;
}
