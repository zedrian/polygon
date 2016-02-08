#include "message.h"

Message::Message(Header header,
                 const vector<unsigned char>& data)
{
    _bytes.resize(sizeof(Header) + data.size());

    *reinterpret_cast<Header*>(&_bytes.front()) = header;
    for (size_t i = 0; i < data.size(); ++i)
        _bytes[sizeof(Header) + i] = data[i];
}

Message::Message(vector<unsigned char>& bytes) :
        _bytes(bytes)
{ }


bool Message::operator<(const Message& other) const
{
    return header().id() > other.header().id();
}

Header Message::header() const
{
    return *reinterpret_cast<const Header*>(&_bytes[0]);
}

vector<unsigned char> Message::data() const
{
    return vector<unsigned char>(_bytes.end() - _bytes.size() + sizeof(Header), _bytes.end());
}

vector<unsigned char>& Message::bytes()
{
    return _bytes;
}