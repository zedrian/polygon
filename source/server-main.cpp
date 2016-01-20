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


void processClientInput(vector<unsigned char>& input,
                        vector<unsigned char>& result)
{
    result = vector<unsigned char>(3, 0x00);
    unsigned char minimum = input[0], maximum = input[0], sum = 0;

    for(auto& x : input)
    {
        if (x < minimum)
            minimum = x;
        if (x > maximum)
            maximum = x;
        sum += x;
    }

    result[0] = minimum;
    result[1] = maximum;
    result[2] = sum;
}

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
        bytes_received = incoming_socket->receive(buf, 0);
        cout << "success" << endl;

        sending_data = vector<unsigned char>(bytes_received);
        cout << "Received from client (" << dec << bytes_received << " bytes): ";
        for (int i = 0; i < bytes_received; ++i)
        {
            unsigned short x = buf[i];
            cout << hex << x << " ";
        }
        cout << endl;

        processClientInput(buf, sending_data);

        cout << "Sending to client: ";
        bytes_sent = incoming_socket->send(sending_data);
        cout << "success" << endl;
        cout << "Sent to client (" << dec << bytes_sent << " bytes): ";
        for (int i = 0; i < bytes_sent; ++i)
        {
            unsigned short x = sending_data[i];
            cout << hex << x << " ";
        }
        cout << endl;

        cout << "Closing the connection: ";
        incoming_socket->close();
        cout << "success" << endl;
    }
    while (true);
}


int main(int argc,
         char** argv)
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
