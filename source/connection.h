#pragma once


#include <memory>

#include "socket.h"


using std::shared_ptr;


class Connection
{
public:
    Connection(string address,
               unsigned short port);
    Connection(shared_ptr<Socket> socket_from_acceptor);
    ~Connection();

    bool connected() const;
    void close();

    shared_ptr<Socket> socket() const;

    void send(vector<unsigned char>& data);
    vector<unsigned char> receive(unsigned long timeout_in_milliseconds = 0);


private:
    shared_ptr<Socket> _socket;
};