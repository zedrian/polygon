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
    Socket(mbedtls_net_context net_context,
           mbedtls_ssl_context ssl_context);
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

    size_t maximumFragmentSize() const;

    void generateRandom(unsigned char* buffer,
                        size_t size);


private:
    mbedtls_net_context _net_context;
    mbedtls_ssl_context _ssl_context;
    bool _constructed_by_acceptor;

    mbedtls_x509_crt _certificate;
    mbedtls_ssl_config _ssl_configuration;
    mbedtls_entropy_context _entropy_context;
    mbedtls_ctr_drbg_context _drbg_context;
    mbedtls_timing_delay_context _delay_context;

    bool _active;
};