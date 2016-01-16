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
    mbedtls_net_context listen_fd;
    mbedtls_x509_crt srvcert;
    mbedtls_ssl_config conf;
    mbedtls_pk_context pkey;
    mbedtls_ssl_cookie_ctx cookie_ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_cache_context cache;
    mbedtls_timing_delay_context timer;

    mbedtls_net_context client_fd;
    mbedtls_ssl_context ssl;
    unsigned char client_ip[16] = {0};
    size_t cliip_len;
};