#pragma once


#include <cstdint>
#include <string>
#include <vector>

#include <mbedtls/net.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/timing.h>
#include <mbedtls/debug.h>
#include <mbedtls/certs.h>
#include <mbedtls/error.h>


using std::size_t;
using std::string;
using std::vector;


/*
 * Socket interface :
 *  size_t& sendTimeout() ?
 *  size_t& receiveTimeout() ?
 *  size_t send(const unsigned char* data, size_t size)
 *  size_t receive(unsigned char* data, size_t maximum_size)
 *  size_t pendingDatagramSize() ?
 *  bool hasPendingDatagams() ?
 */


class Socket
{
public:
    Socket();
    ~Socket();

    void connect(const string address,
                 unsigned short port);
    void close();

    size_t send(const unsigned char* data,
                size_t size);
    size_t send(const vector<unsigned char>& data);

    size_t receive(unsigned char* buffer,
                   size_t maximum_size);
    size_t receive(vector<unsigned char>& buffer);

    vector<unsigned char> sendWithConfirmation(const vector<unsigned char>& data);


private:
    mbedtls_net_context server_fd;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    size_t maximum_fragment_size;
    bool active;
};