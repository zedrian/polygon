#include <stdexcept>
#include <condition_variable>

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
    Header header(MessageType::Normal, ++_last_sent_message_id, mbedtls_timing_get_timer(&_clock, 0));
    Message message(header, data);

    _socket->send(message.bytes());
}

vector<unsigned char> Connection::receive(unsigned long timeout_in_milliseconds)
{
    unique_lock<mutex> lock(_messages_mutex);
    condition_variable messages_not_empty_condition;
    messages_not_empty_condition.wait_for(lock, std::chrono::milliseconds(timeout_in_milliseconds), [&]{return !_messages.empty();});

    if(_messages.empty())
        return vector<unsigned char>();

    auto data = _messages.front().data();
    _messages.pop();

    return data;
}

void Connection::setWhenReceiveLambda(Connection::WhenReceiveLambda lambda)
{
    _when_receive_lambda = lambda;
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

        while(connected())
        {
            buffer.resize(maximumMessageSize(), 0x00);
            if(_socket->receive(buffer) == 0)
                continue;

            if(_when_receive_lambda)
                _when_receive_lambda(Message(buffer).data());
            else
            {
                unique_lock<mutex> lock(_messages_mutex);
                _messages.push(Message(buffer));
            }
        }
    });
    _receiver.detach();
}
