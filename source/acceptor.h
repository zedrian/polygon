#pragma once


#include <memory>

#include <mbedtls/config.h>
#include <mbedtls/ssl_cookie.h>
#include <mbedtls/ssl_cache.h>

#include "socket.h"


using std::shared_ptr;


class Acceptor
{
public:
    Acceptor();
    ~Acceptor();

    void listen(string address,
                unsigned short port);
    shared_ptr<Socket> accept();
    void close();


private:
    mbedtls_net_context _net_context;
    mbedtls_x509_crt _certificate;
    mbedtls_ssl_config _ssl_configuration;
    mbedtls_pk_context _public_key_context;
    mbedtls_ssl_cookie_ctx _cookie_context;
    mbedtls_entropy_context _enthropy_context;
    mbedtls_ctr_drbg_context _drbg_context;
    mbedtls_ssl_cache_context _cache_context;
    mbedtls_timing_delay_context _delay_context;

    mbedtls_net_context _incoming_net_context;
    mbedtls_ssl_context _incoming_ssl_context;
};