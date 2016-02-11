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
    shared_ptr<Connection> connection;
    string address;
    unsigned int port;
    vector<unsigned char> input(2, 0x00);
    vector<unsigned char> world_state(2, 0x00);

    {
        cout << "Enter server IP: ";
        cin >> address;
        cout << "Enter server port: ";
        cin >> port;

        cout << "Connecting to server: ";
        connection = make_shared<Connection>(address, port);
        connection->configurePing(1000, 3);
        connection->setWhenReceiveLambda([](vector<unsigned char> world_state)
        {
           cout << endl << "World state received: " << *reinterpret_cast<unsigned short*>(world_state.data()) << endl;
        });
        cout << "success" << endl;
        cout << "Maximum size of a fragment for current session: " << connection->maximumMessageSize() << endl;

        while (connection->connected())
        {
            cout << "Generating user input: ";
            connection->generateRandom(input.data(), 2);
            input[1] = 0x00;
            cout << dec << *reinterpret_cast<unsigned short*>(input.data()) << endl;
            cout << "Sending input to server: ";
            connection->send(input);
            cout << "success" << endl;

            connection->generateRandom(input.data(), 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(input[0] % 40));
        }
    }
    cout << "Exiting." << endl;
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
        cout << "Fail: " << e.what() << endl;
    }
}