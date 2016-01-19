#include <iostream>

#include "stupid.h"
#include "socket.h"


using namespace polygon::math;
using namespace polygon::network;

using std::cout;
using std::endl;


int main()
{
    float a = 2.0f;
    float b = 3.0f;

    cout << "a = 2.0" << endl;
    cout << "b = 3.0" << endl;
    cout << "stupidMin(a, b) = " << stupidMin(a, b) << endl;
    cout << "stupidMax(a, b) = " << stupidMax(a, b) << endl;

    vector<unsigned char> data(100, 0x00);

    Socket socket;
    socket.connect("127.0.0.1", 27182);
    cout << "socket.send() = " << socket.send(data) << endl;
    cout << "socket.receive() = " << socket.receive(data) << endl;
    socket.close();
}
