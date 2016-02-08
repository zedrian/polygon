#pragma once


#include <vector>

#include "header.h"


using std::vector;
using std::size_t;


class Message
{
public:
    Message(Header header,
            const vector<unsigned char>& data);
    Message(vector<unsigned char>& bytes);

    bool operator<(const Message& other) const;

    Header header() const;
    vector<unsigned char> data() const;
    vector<unsigned char>& bytes();


private:
    vector<unsigned char> _bytes;
};