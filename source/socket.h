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

#include "message.h"


using std::size_t;
using std::string;
using std::vector;


class Socket
{
public:
    Socket();
    Socket(mbedtls_net_context net_context,
           mbedtls_ssl_context ssl_context,
           mbedtls_ssl_config ssl_configuration);
    ~Socket();

    void connect(const string address,
                 unsigned short port);
    void close();
    bool connected() const;

    size_t send(const unsigned char* data,
                size_t size);
    size_t send(const vector<unsigned char>& data);

    size_t receive(unsigned char *buffer,
                   size_t maximum_size,
                   unsigned long timeout_in_milliseconds = 0);
    size_t receive(vector<unsigned char> &buffer,
                   unsigned long timeout_in_milliseconds = 0);

    size_t maximumFragmentSize() const;

    void generateRandom(unsigned char* buffer,
                        size_t size);


private:
    size_t sendMessage(Message& message);
    size_t receiveMessage(Message& message,
                          unsigned long timeout_in_milliseconds);


private:
    mbedtls_net_context _net_context;
    mbedtls_ssl_context _ssl_context;
    bool _constructed_by_acceptor;

    mbedtls_x509_crt _certificate;
    mbedtls_ssl_config _ssl_configuration;
    mbedtls_entropy_context _entropy_context;
    mbedtls_ctr_drbg_context _ctr_drbg_context;
    mbedtls_timing_delay_context _delay_context;

    size_t _last_sent_message_id;
    size_t _last_received_message_id;
    mbedtls_timing_hr_time _clock;
    bool _connected;
};