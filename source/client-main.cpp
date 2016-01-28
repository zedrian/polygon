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


unsigned short chooseMenuAction()
{
    cout << "Choose an action:" << endl;
    cout << "1. Connect to server" << endl;
    cout << "2. Generate new data" << endl;
    cout << "3. Send data to server" << endl;
    cout << "4. Receive data from server" << endl;
    cout << "0. Exit" << endl;
    cout << endl;
    cout << "Your choice: ";

    unsigned short choice;
    cin >> choice;
    return choice;
}

void work()
{
    shared_ptr<Connection> connection;
    string address;
    unsigned int port;
    vector<unsigned char> data;
    vector<unsigned char> server_data;
    unsigned char data_size;

    unsigned short action = 1;
    while (action != 0)
    {
        action = chooseMenuAction();

        switch (action)
        {
            case 1:
                cout << "Enter server IP: ";
                cin >> address;
                cout << "Enter server port: ";
                cin >> port;

                cout << "Connecting to server: ";
                connection = make_shared<Connection>(address, port);
                cout << "success" << endl;
                cout << "Maximum size of a fragment for current session: " << connection->maximumMessageSize() << endl;
                break;

            case 2:
                connection->generateRandom(&data_size, 1);
                data.resize(data_size);
                connection->generateRandom(data.data(), data_size);
                cout << "Generated data (" << dec << data.size() << " bytes):" << endl;
                showArray(data);
                break;

            case 3:
                cout << "Sending data to server: ";
                connection->send(data);
                cout << "success" << endl;
                break;

            case 4:
                cout << "Enter timeout in milliseconds (0 for waiting forever): ";
                unsigned long timeout;
                cin >> timeout;

                cout << "Receiving data from server: ";
                server_data = connection->receive(timeout);
                if (server_data.size() != 0)
                {
                    cout << "success" << endl;
                    cout << "Received from server (" << dec << server_data.size() << " bytes):" << endl;
                    showArray(server_data);
                    break;
                }
                if(connection->connected())
                {
                    cout << "timeout" << endl;
                    break;
                }

            case 0:
                cout << "Exiting." << endl;
                break;

            default:
                cout << "Unrecognized action. Exiting." << endl;
                break;
        }
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