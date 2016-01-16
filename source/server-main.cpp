#include <iomanip>
#include <iostream>

#include "acceptor.h"


using std::vector;
using std::cin;
using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::runtime_error;
using std::exception;
using std::shared_ptr;
using std::make_shared;


void work()
{
    Acceptor acceptor;
    size_t bytes_sent, bytes_received;

    vector<unsigned char> buf(1024, 0x00);
    vector<unsigned char> sending_data;

    cout << "Enter listening address: ";
    string listening_address;
    cin >> listening_address;
    cout << "Enter port: ";
    unsigned short port;
    cin >> port;
    acceptor.listen(listening_address, port);

    do
    {
        auto incoming_socket = acceptor.accept();
        cout << "Maximum size of a fragment for current session: " << incoming_socket->maximumFragmentSize() << endl;

        cout << "Receiving from client: ";
        bytes_received = incoming_socket->receive(buf);
        cout << "success" << endl;

        sending_data = vector<unsigned char>(bytes_received);
        cout << "Received from client (" << dec << bytes_received << " bytes): ";
        for (int i = 0; i < bytes_received; ++i)
        {
            sending_data[i] = buf[i];
            unsigned short x = buf[i];
            cout << hex << x << " ";
        }
        cout << endl;

        cout << "Sending to client: ";
        bytes_sent = incoming_socket->send(sending_data);
        cout << "success" << endl;
        cout << "Sent to client (" << dec << bytes_sent << " bytes): ";
        for (int i = 0; i < bytes_sent; ++i)
        {
            unsigned short x = buf[i];
            cout << hex << x << " ";
        }
        cout << endl;

        cout << "Closing the connection: ";
        incoming_socket->close();
        cout << "success" << endl;
    }
    while(true);
}


int main(int argc, char** argv)
{
    try
    {
        work();
    }
    catch (exception& e)
    {
        cout << "Fail: " << e.what() << endl;
    }
}
