#include <iomanip>
#include <iostream>

#include "acceptor.h"
#include "utilities.h"


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

    vector<unsigned char> buffer(1024, 0x00);
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
        buffer.resize(incoming_socket->maximumFragmentSize());
        bytes_received = incoming_socket->receive(buffer, 100);
        if(bytes_received == 0)
        {
            cout << "Closing the connection." << endl;
            continue;
        }

        buffer.resize(bytes_received);
        cout << "success" << endl;

        sending_data = vector<unsigned char>(bytes_received);
        cout << "Received from client (" << dec << bytes_received << " bytes):" << endl;
        showArray(buffer);

        processClientInput(buffer, sending_data);

        cout << "Sending to client: ";
        bytes_sent = incoming_socket->send(sending_data);
        cout << "success" << endl;
        cout << "Sent to client (" << dec << bytes_sent << " bytes):" << endl;
        showArray(sending_data);

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
