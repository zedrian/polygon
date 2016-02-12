#include <iomanip>
#include <iostream>

#include "acceptor.h"
#include "connection.h"
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


void processClientInput(unsigned short& input,
                        unsigned short& world_state)
{
    world_state += input;
}

void work()
{
    Acceptor acceptor;

    cout << "Enter listening address: ";
    string listening_address;
    cin >> listening_address;
    cout << "Enter port: ";
    unsigned short port;
    cin >> port;
    acceptor.listen(listening_address, port);

    cout << "Waiting for remote connection: ";
    Connection connection(acceptor.accept());
    connection.configurePing(1000, 3);
    cout << "success" << endl;
    cout << "Maximum size of a fragment for current session: " << connection.maximumMessageSize() << endl;

    vector<unsigned char> world_state_serialized(2, 0x00);
    unsigned short client_input;
    unsigned short world_state = 0;
    while(connection.connected())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cout << "Waking up." << endl;
        auto all_inputs_serialized = connection.getAllReceived();
        for (auto& input : all_inputs_serialized)
        {
            client_input = *reinterpret_cast<unsigned short*>(input.data());
            processClientInput(client_input, world_state);
        }
        cout << "World state: " << dec << world_state << " (" << all_inputs_serialized.size() << " inputs processed)" << endl;
        cout << "Sending to client: ";
        *reinterpret_cast<unsigned short*>(world_state_serialized.data()) = world_state;
        connection.send(world_state_serialized);
        cout << "success" << endl;
    }

    cout << "Exiting." << endl;
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
