#include <stdexcept>

#include "message.h"
#include "connection.h"


using std::make_shared;
using std::logic_error;
using std::to_string;
using std::runtime_error;
using std::this_thread::sleep_for;


Connection::Connection(string address,
                       unsigned short port)
{
    _socket = make_shared<Socket>();
    _socket->connect(address, port);

    if (!_socket->connected())
        throw runtime_error("Failed to establish connection to " + address + ":" + to_string(port) + ".");

    mbedtls_timing_get_timer(&_clock, 1);
    _last_sent_message_id = 0;
}

Connection::Connection(shared_ptr<Socket> socket_from_acceptor) :
        _socket(socket_from_acceptor)
{
    if (_socket == nullptr)
        throw logic_error("Connection can not be initialized without correct socket.");

    if (!_socket->connected())
        throw logic_error("Connection can not be initialized with closed socket.");

    mbedtls_timing_get_timer(&_clock, 1);
    _last_sent_message_id = 0;
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
    vector<unsigned char> buffer(_socket->maximumFragmentSize(), 0x00);

    if(_socket->receive(buffer, timeout_in_milliseconds) == 0)
        return vector<unsigned char>();

    Message message(buffer);
    _last_received_message_id = message.header().id();

    return message.data();
}

void Connection::setWhenReceiveLambda(Connection::WhenReceiveLambda lambda)
{
    _when_receive_lambda = lambda;

    _for_receive_waiter = thread([&]()
     {
         vector<unsigned char> data;

         while (_socket->connected())
         {
             data = receive(10);
             if (!data.empty())
                 _when_receive_lambda(data);
         }
     });
    _for_receive_waiter.detach();
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
