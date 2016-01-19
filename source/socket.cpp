#include "socket-implementation.h"


using namespace polygon::network;

using std::make_shared;


Socket::Socket()
{
    _pimpl = make_shared<Implementation>();
}

Socket::~Socket()
{ }


void Socket::connect(string address,
                     unsigned short port)
{
    return _pimpl->connect(address, port);
}

void Socket::close()
{
    return _pimpl->close();
}

size_t Socket::send(vector<unsigned char>& data)
{
    return _pimpl->send(data);
}

size_t Socket::receive(vector<unsigned char>& buffer)
{
    return _pimpl->receive(buffer);
}