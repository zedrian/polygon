#pragma once


#include <memory>
#include <thread>
#include <queue>
#include <mutex>

#include "message.h"
#include "socket.h"


using std::shared_ptr;
using std::thread;
using std::queue;
using std::mutex;


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
    void initialize();

    void send(MessageType type,
              vector<unsigned char>& data);


private:
    shared_ptr<Socket> _socket;

    thread _receiver;
    WhenReceiveLambda _when_receive_lambda;

    queue<Message> _messages;
    mutex _messages_mutex;

    size_t _last_sent_message_id;
    size_t _last_received_message_id;
    mbedtls_timing_hr_time _clock;
};