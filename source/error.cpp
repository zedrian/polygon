#include <mbedtls/error.h>

#include "error.h"


string constructErrorMessage(string command,
                             int code)
{
    char buffer[100];
    mbedtls_strerror(code, buffer, 100);

    return command + " failed with error code " + to_string(code) + " - " + buffer;
}