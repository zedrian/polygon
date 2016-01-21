#pragma once


#include <vector>
#include <string>


using std::vector;
using std::string;


void showArray(vector<unsigned char>& data);

string addressToString(unsigned char* address,
                       int address_length);