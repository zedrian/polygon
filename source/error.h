#pragma once


#include <iostream>
#include <string>


using std::cout;
using std::endl;
using std::string;
using std::to_string;


static void simpleDebug(void* ctx, int level,
                     const char* file, int line,
                     const char* str)
{
    cout << file << ":" << line << ": " << str << endl;
}

string constructErrorMessage(string command,
                             int code);