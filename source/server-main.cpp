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


void processClientInput(vector<unsigned char>& input,
                        vector<unsigned char>& result)
{
    unsigned char minimum = input[0], maximum = input[0], sum = 0;

    for (auto& x : input)
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

    cout << "Enter listening address: ";
    string listening_address;
    cin >> listening_address;
    cout << "Enter port: ";
    unsigned short port;
    cin >> port;
    acceptor.listen(listening_address, port);

    cout << "Waiting for remote connection: ";
    Connection connection(acceptor.accept());
    cout << "success" << endl;
    cout << "Maximum size of a fragment for current session: " << connection.maximumMessageSize() << endl;

    vector<unsigned char> sending_data(3, 0x00);
    connection.setWhenReceiveLambda([&](vector<unsigned char> client_input)
                                    {
                                        cout << "Received from client (" << dec << client_input.size() << " bytes):" << endl;
                                        showArray(client_input);

                                        processClientInput(client_input, sending_data);

                                        cout << "Sending to client: ";
                                        connection.send(sending_data);
                                        cout << "success" << endl;
                                        cout << "Sent to client:" << endl;
                                        showArray(sending_data);
                                    });

    while(connection.connected());
    cout << "Connection closed. Exiting." << endl;
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
