#include <iomanip>
#include <stdexcept>

#include "error.h"
#include "acceptor.h"
#include "utilities.h"


using std::make_shared;
using std::dec;
using std::runtime_error;


Acceptor::Acceptor()
{
    int ret;

    mbedtls_net_init(&_net_context);
    mbedtls_ssl_config_init(&_ssl_configuration);
    mbedtls_ssl_cookie_init(&_cookie_context);
    mbedtls_ssl_cache_init(&_cache_context);
    mbedtls_x509_crt_init(&_certificate);
    mbedtls_pk_init(&_public_key_context);
    mbedtls_entropy_init(&_entropy_context);
    mbedtls_ctr_drbg_init(&_ctr_drbg_context);
    mbedtls_debug_set_threshold(0);

    cout << "Loading server certificate and key: ";
    /*
     * This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ret = mbedtls_x509_crt_parse(&_certificate, (const unsigned char*) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len);
    if (ret != 0)
        throw runtime_error(constructErrorMessage("mbedtls_x509_crt_parse()", ret));

    ret = mbedtls_x509_crt_parse(&_certificate, (const unsigned char*) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (ret != 0)
        throw runtime_error(constructErrorMessage("mbedtls_x509_crt_parse()", ret));

    ret = mbedtls_pk_parse_key(&_public_key_context, (const unsigned char*) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0);
    if (ret != 0)
        throw runtime_error(constructErrorMessage("mbedtls_pk_parse_key()", ret));

    cout << "success" << endl;


    cout << "Seeding the random number generator: ";
    string seeding_vector = "dtls_server";
    if ((ret = mbedtls_ctr_drbg_seed(&_ctr_drbg_context, mbedtls_entropy_func, &_entropy_context, (const unsigned char*) seeding_vector.data(), seeding_vector.size())) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ctr_drbg_seed()", ret));
    cout << "success" << endl;

    cout << "Setting up the DTLS data: ";
    if ((ret = mbedtls_ssl_config_defaults(&_ssl_configuration, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ssl_config_defaults()", ret));

    mbedtls_ssl_conf_rng(&_ssl_configuration, mbedtls_ctr_drbg_random, &_ctr_drbg_context);
    mbedtls_ssl_conf_dbg(&_ssl_configuration, simpleDebug, stdout);

    mbedtls_ssl_conf_session_cache(&_ssl_configuration, &_cache_context, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);

    mbedtls_ssl_conf_ca_chain(&_ssl_configuration, _certificate.next, NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&_ssl_configuration, &_certificate, &_public_key_context)) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ssl_conf_own_cert()", ret));

    if ((ret = mbedtls_ssl_cookie_setup(&_cookie_context, mbedtls_ctr_drbg_random, &_ctr_drbg_context)) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ssl_cookie_setup()", ret));

    mbedtls_ssl_conf_dtls_cookies(&_ssl_configuration, mbedtls_ssl_cookie_write, mbedtls_ssl_cookie_check, &_cookie_context);

    cout << "success" << endl;
}

Acceptor::~Acceptor()
{
    close();
}

void Acceptor::listen(string address,
                      unsigned short port)
{
    cout << "Binding on UDP: ";

    int ret;
    if ((ret = mbedtls_net_bind(&_net_context, address.data(), to_string(port).data(), MBEDTLS_NET_PROTO_UDP)) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_net_bind()", ret));

    cout << "success" << endl;
}

shared_ptr<Socket> Acceptor::accept()
{
    int ret;
    mbedtls_net_context incoming_net_context;
    mbedtls_ssl_context incoming_ssl_context;

    cout << "Preparing SSL context for remote connection: ";
    mbedtls_ssl_init(&incoming_ssl_context);
    if ((ret = mbedtls_ssl_setup(&incoming_ssl_context, &_ssl_configuration)) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ssl_setup()", ret));
    mbedtls_ssl_set_timer_cb(&incoming_ssl_context, &_delay_context, mbedtls_timing_set_delay, mbedtls_timing_get_delay);
    cout << "success" << endl;

    unsigned char client_address[16] = {0};
    size_t client_address_length;

    do
    {
        cout << "Waiting for a remote connection: ";
        mbedtls_net_free(&incoming_net_context);
        mbedtls_ssl_session_reset(&incoming_ssl_context);

        if ((ret = mbedtls_net_accept(&_net_context, &incoming_net_context, client_address, sizeof(client_address), &client_address_length)) != 0)
            throw runtime_error(constructErrorMessage("mbedtls_net_accept()", ret));

        /* For HelloVerifyRequest cookies */
        if ((ret = mbedtls_ssl_set_client_transport_id(&incoming_ssl_context, client_address, client_address_length)) != 0)
            throw runtime_error(constructErrorMessage("mbedtls_ssl_set_client_transport_id()", ret));

        mbedtls_ssl_set_bio(&incoming_ssl_context, &incoming_net_context, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
        cout << "success" << endl;

        cout << "Performing the DTLS handshake: ";
        do
            ret = mbedtls_ssl_handshake(&incoming_ssl_context);
        while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

        if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED)
        {
            cout << "hello verification requested" << endl;
            continue;
        }
        else if (ret != 0)
        {
            cout << constructErrorMessage("mbedtls_ssl_handshake()", ret) << endl;
            continue;
        }

        break;
    }
    while (true);
    cout << "success" << endl;

    cout << "Client address: " << addressToString(client_address, client_address_length) << endl;

    return make_shared<Socket>(incoming_net_context, incoming_ssl_context, _ssl_configuration);
}

void Acceptor::close()
{
    mbedtls_net_free(&_net_context);
    mbedtls_x509_crt_free(&_certificate);
    mbedtls_pk_free(&_public_key_context);
    mbedtls_ssl_config_free(&_ssl_configuration);
    mbedtls_ssl_cookie_free(&_cookie_context);
    mbedtls_ssl_cache_free(&_cache_context);
    mbedtls_ctr_drbg_free(&_ctr_drbg_context);
    mbedtls_entropy_free(&_entropy_context);
}
