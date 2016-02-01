#include <stdexcept>
#include <condition_variable>
#include <iostream>

#include "connection.h"


using std::make_shared;
using std::logic_error;
using std::to_string;
using std::runtime_error;
using std::this_thread::sleep_for;
using std::unique_lock;
using std::condition_variable;


Connection::Connection(string address,
                       unsigned short port)
{
    _socket = make_shared<Socket>();
    _socket->connect(address, port);

    if (!_socket->connected())
        throw runtime_error("Failed to establish connection to " + address + ":" + to_string(port) + ".");

    initialize();
}

Connection::Connection(shared_ptr<Socket> socket_from_acceptor) :
        _socket(socket_from_acceptor)
{
    if (_socket == nullptr)
        throw logic_error("Connection can not be initialized without correct socket.");

    if (!_socket->connected())
        throw logic_error("Connection can not be initialized with closed socket.");

    initialize();
}

Connection::~Connection()
{
    close();
}

bool Connection::connected() const
{
    return _socket->connected();
}

void Connection::close()
{
    _socket->close();
}

void Connection::send(vector<unsigned char>& data)
{
    send(MessageType::Normal, data);
}

vector<unsigned char> Connection::receive(unsigned long timeout_in_milliseconds)
{
    unique_lock<mutex> lock(_messages_mutex);
    condition_variable messages_not_empty_condition;
    messages_not_empty_condition.wait_for(lock, std::chrono::milliseconds(timeout_in_milliseconds), [&]
    { return !_messages.empty(); });

    if (_messages.empty())
        return vector<unsigned char>();

    auto data = _messages.front().data();
    _messages.pop();

    return data;
}

void Connection::setWhenReceiveLambda(Connection::WhenReceiveLambda lambda)
{
    _when_receive_lambda = lambda;
}

void Connection::configurePing(unsigned long ping_interval_in_milliseconds,
                               unsigned char maximum_ping_messages_loss)
{
    _pinger = thread([&]
    {
        unsigned char iteration_index = 0;
        vector<unsigned char> ping_data = {0x27, 0x18, 0x28};

        while (connected())
        {
            sleep_for(std::chrono::milliseconds(ping_interval_in_milliseconds));
            send(MessageType::Ping, ping_data);
            std::cout << "Ping sent." << std::endl;

            iteration_index = (iteration_index + 1) % maximum_ping_messages_loss;
            if (iteration_index == 0)
            {
                unique_lock<mutex> lock(_ping_received_mutex);
                if (!_ping_received)
                    throw runtime_error("Connection timeout.");
                else
                    _ping_received = false;
            }
        }
    });
}

void Connection::generateRandom(unsigned char* buffer,
                                size_t size)
{
    _socket->generateRandom(buffer, size);
}

size_t Connection::maximumMessageSize() const
{
    return _socket->maximumFragmentSize() - sizeof(Header);
}

void Connection::initialize()
{
    mbedtls_timing_get_timer(&_clock, 1);
    _last_sent_message_id = 0;

    _receiver = thread([&]
    {
        vector<unsigned char> buffer;
        unique_lock<mutex> ping_received_lock(_ping_received_mutex);
        ping_received_lock.unlock();

        while (connected())
        {
            buffer.resize(maximumMessageSize(), 0x00);
            if (_socket->receive(buffer) == 0)
                continue;

            Message message_received(buffer);
            switch(message_received.header().type())
            {
                case MessageType::Normal:
                    if (_when_receive_lambda)
                        _when_receive_lambda(Message(buffer).data());
                    else
                    {
                        unique_lock<mutex> lock(_messages_mutex);
                        _messages.push(Message(buffer));
                    }
                    break;

                case MessageType::Ping:
                    ping_received_lock.lock();
                    _ping_received = true;
                    std::cout << std::endl << "Ping: " << mbedtls_timing_get_timer(&_clock, 0) - message_received.header().construction_time() << std::endl;
                    ping_received_lock.unlock();
                    break;

                default:
                    throw logic_error("Message of unknown type received.");
            }
        }
    });
    _receiver.detach();
}

void Connection::send(MessageType type,
                      vector<unsigned char>& data)
{
    Header header(type, ++_last_sent_message_id, mbedtls_timing_get_timer(&_clock, 0));
    Message message(header, data);

    _socket->send(message.bytes());
}
