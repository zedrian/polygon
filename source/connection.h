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

    void send(vector<unsigned char>& data);
    vector<unsigned char> receive(unsigned long timeout_in_milliseconds = 0);

    void setWhenReceiveLambda(WhenReceiveLambda lambda);

    void generateRandom(unsigned char* buffer,
                        size_t size);
    size_t maximumMessageSize() const;


private:
    shared_ptr<Socket> _socket;

    thread _for_receive_waiter;
    WhenReceiveLambda _when_receive_lambda;

    size_t _last_sent_message_id;
    size_t _last_received_message_id;
    mbedtls_timing_hr_time _clock;
};