#pragma once


#include <memory>
#include <thread>

#include "socket.h"


using std::shared_ptr;
using std::thread;


class Connection
{
public:
    using WhenReceiveLambda = std::function<void(vector<unsigned char> data)>;


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

    void setWhenReceiveLambda(WhenReceiveLambda lambda);


private:
    shared_ptr<Socket> _socket;

    thread _for_receive_waiter;
    WhenReceiveLambda _when_receive_lambda;
};