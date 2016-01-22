#pragma once


#include <vector>

#include "header.h"


using std::vector;
using std::size_t;


class Message
{
public:
    Message(Header header,
            const unsigned char* data,
            size_t size);
    Message(size_t total_size);

    Header header() const;
    unsigned char* data();
    vector<unsigned char>& bytes();


private:
    vector<unsigned char> _bytes;
};