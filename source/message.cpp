#include "message.h"

Message::Message(Header header,
                 const unsigned char* data,
                 size_t size)
{
    _bytes.resize(sizeof(Header) + size);

    *reinterpret_cast<Header*>(&_bytes.front()) = header;
    for(size_t i = 0; i < size; ++i)
        _bytes[sizeof(Header) + i] = data[i];
}

Message::Message(size_t total_size)
{
    _bytes.resize(total_size);
}

Header Message::header() const
{
    return *reinterpret_cast<const Header*>(&_bytes[0]);
}

unsigned char* Message::data()
{
    return &_bytes[sizeof(Header)];
}

vector<unsigned char>& Message::bytes()
{
    return _bytes;
}