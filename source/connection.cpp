#include <stdexcept>

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
}

Connection::Connection(shared_ptr<Socket> socket_from_acceptor) :
        _socket(socket_from_acceptor)
{
    if (_socket == nullptr)
        throw logic_error("Connection can not be initialized without correct socket.");

    if (!_socket->connected())
        throw logic_error("Connection can not be initialized with closed socket.");
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
    _socket->send(data);
}

vector<unsigned char> Connection::receive(unsigned long timeout_in_milliseconds)
{
    vector<unsigned char> buffer(_socket->maximumFragmentSize(), 0x00);

    auto bytes_received = _socket->receive(buffer, timeout_in_milliseconds);
    buffer.resize(bytes_received);

    return buffer;
}

void Connection::setWhenReceiveLambda(Connection::WhenReceiveLambda lambda)
{
    _when_receive_lambda = lambda;

    _for_receive_waiter = thread([&]()
                                 {
                                     vector<unsigned char> buffer;
                                     size_t bytes_received;

                                     while (_socket->connected())
                                     {
                                         buffer.resize(_socket->maximumFragmentSize());
                                         bytes_received = _socket->receive(buffer, 10);
                                         if (bytes_received != 0)
                                         {
                                             buffer.resize(bytes_received);
                                             _when_receive_lambda(buffer);
                                         }
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
