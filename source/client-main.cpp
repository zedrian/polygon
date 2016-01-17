#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "socket.h"


using std::cin;
using std::cout;
using std::endl;
using std::to_string;
using std::hex;
using std::dec;
using std::setfill;
using std::setw;
using std::exception;


void showArray(vector<unsigned char>& data)
{
    for(size_t index = 0; index < data.size(); ++index)
    {
        cout << setfill('0') << hex << setw(2) << static_cast<unsigned short>(data[index]) << ' ';
        ++index;

        if(index % 4 == 0)
            cout << ' ';
        if(index % 8 == 0)
            cout << ' ';
        if(index % 16 == 0)
            cout << endl;

        --index;
    }
    cout << endl;
}

void work()
{
    Socket socket;

    string server_address;
    unsigned int port;
    cout << "Enter server IP: ";
    cin >> server_address;
    cout << "Enter server port: ";
    cin >> port;

    socket.connect(server_address, port);
    cout << "Maximum size of a fragment for current session: " << socket.maximumFragmentSize() << endl;

    unsigned char data_size;
    socket.generateRandom(&data_size, 1);
    vector<unsigned char> data(data_size, 0x00);
    socket.generateRandom(data.data(), data_size);

    cout << "Data to send to server (" << dec << data.size() << " bytes):" << endl;
    showArray(data);

    socket.send(data);
    vector<unsigned char> response(socket.maximumFragmentSize(), 0x00);
    auto bytes_received = socket.receive(response);
    response.resize(bytes_received);

    cout << "Server's response (" << dec << response.size() << " bytes):" << endl;
    showArray(response);

    cout << "Closing the connection: ";
    socket.close();
    cout << "success" << endl;
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