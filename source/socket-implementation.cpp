#include <iostream>

#include "socket-implementation.h"


using namespace polygon::network;

using std::cout;
using std::endl;


Socket::Implementation::Implementation()
{
    cout << "Socket::Implementation()" << endl;
    _net_context = 2;
    _ssl_context = 19;
}


Socket::Implementation::~Implementation()
{
    cout << "Socket::~Implementation()" << endl;
}

void Socket::Implementation::connect(string address,
                                     unsigned short port)
{
    cout << "Socket::Implementation::connect(" << address << ", " << port << ")" << endl;
}

void Socket::Implementation::close()
{
    cout << "Socket::Implementation::close()" << endl;
}

size_t Socket::Implementation::send(vector<unsigned char>& data)
{
    cout << "Socket::Implementation::send()" << endl;
    return _net_context;
}

size_t Socket::Implementation::receive(vector<unsigned char>& buffer)
{
    cout << "Socket::Implementation::receive()" << endl;
    return _ssl_context;
}
