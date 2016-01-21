#include <iostream>
#include <iomanip>
#include <sstream>

#include "utilities.h"


using std::size_t;
using std::cout;
using std::setfill;
using std::hex;
using std::dec;
using std::setw;
using std::endl;
using std::stringstream;


void showArray(vector<unsigned char>& data)
{
    for (size_t index = 0; index < data.size(); ++index)
    {
        cout << setfill('0') << hex << setw(2) << static_cast<unsigned short>(data[index]) << ' ';

        if (index % 4 == 3)
            cout << ' ';
        if (index % 8 == 7)
            cout << ' ';
        if (index % 16 == 15)
            cout << endl;
    }
    cout << endl;
}

string addressToString(unsigned char* address,
                       int address_length)
{
    stringstream stream;

    for (unsigned char i = 0; i < address_length - 1; ++i)
        stream << dec << static_cast<unsigned short>(address[i]) << '.';
    stream << dec << static_cast<unsigned short>(address[address_length - 1]);

    return stream.str();
}