#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "connection.h"
#include "utilities.h"


using std::cin;
using std::cout;
using std::endl;
using std::to_string;
using std::hex;
using std::dec;
using std::setfill;
using std::setw;
using std::exception;
using std::make_shared;


void work()
{
    string server_address;
    unsigned int port;
    cout << "Enter server IP: ";
    cin >> server_address;
    cout << "Enter server port: ";
    cin >> port;

    Connection connection(server_address, port);
    cout << "Maximum size of a fragment for current session: " << connection.socket()->maximumFragmentSize() << endl;

    unsigned char data_size;
    connection.socket()->generateRandom(&data_size, 1);
    vector<unsigned char> data(data_size, 0x00);
    connection.socket()->generateRandom(data.data(), data_size);

    cout << "Data to send to server (" << dec << data.size() << " bytes):" << endl;
    showArray(data);

    vector<unsigned char> response;
    for(auto i = 0; i < 9; ++i)
    {
        cout << "Sending data to server: ";
        connection.send(data);
        cout << "success" << endl;

        cout << "Receiving server's response: ";
        response = connection.receive();
        cout << "success" << endl;

        cout << "Server's response (" << dec << response.size() << " bytes):" << endl;
        showArray(response);
    }
}

int main(int argc,
         char* argv[])
{
    try
    {
        work();
    }
    catch (exception& e)
    {
        cout << "fail: " << e.what() << endl;
    }
}