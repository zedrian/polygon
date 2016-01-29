#include <stdexcept>

#include <mbedtls/timing.h>

#include "error.h"
#include "socket.h"


using std::runtime_error;
using std::logic_error;
using std::exception;


Socket::Socket()
{
    mbedtls_debug_set_threshold(0);

    mbedtls_net_init(&_net_context);
    mbedtls_ssl_init(&_ssl_context);
    mbedtls_ssl_config_init(&_ssl_configuration);
    mbedtls_x509_crt_init(&_certificate);
    mbedtls_ctr_drbg_init(&_ctr_drbg_context);
    mbedtls_entropy_init(&_entropy_context);

    string personalizing_vector = "Socket, constructed by itself at the moment = " + to_string(mbedtls_timing_hardclock());
    int result;
    if ((result = mbedtls_ctr_drbg_seed(&_ctr_drbg_context, mbedtls_entropy_func, &_entropy_context, (const unsigned char*) personalizing_vector.data(), personalizing_vector.size())) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ctr_drbg_seed()", result));

    if ((result = mbedtls_x509_crt_parse(&_certificate, (const unsigned char*) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len)) < 0)
        throw runtime_error(constructErrorMessage("mbedtls_x509_crt_parse()", result));

    _constructed_by_acceptor = false;
    _connected = false;
}

Socket::Socket(mbedtls_net_context net_context,
               mbedtls_ssl_context ssl_context,
               mbedtls_ssl_config ssl_configuration)
{
    _net_context = net_context;
    _ssl_context = ssl_context;
    _ssl_configuration = ssl_configuration;
    _ssl_context.conf = &_ssl_configuration;

    mbedtls_ssl_set_bio(&_ssl_context, &_net_context, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    mbedtls_ctr_drbg_init(&_ctr_drbg_context);
    mbedtls_entropy_init(&_entropy_context);
    string personalizing_vector = "Socket, constructed by Acceptor at the moment = " + to_string(mbedtls_timing_hardclock());
    int result;
    if ((result = mbedtls_ctr_drbg_seed(&_ctr_drbg_context, mbedtls_entropy_func, &_entropy_context, (const unsigned char*) personalizing_vector.data(), personalizing_vector.size())) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ctr_drbg_seed()", result));

    _constructed_by_acceptor = true;
    _connected = true;
}

Socket::~Socket()
{
    mbedtls_net_free(&_net_context);
    mbedtls_ssl_free(&_ssl_context);
    mbedtls_ctr_drbg_free(&_ctr_drbg_context);
    mbedtls_entropy_free(&_entropy_context);

    if (!_constructed_by_acceptor)
    {
        mbedtls_ssl_config_free(&_ssl_configuration);
        mbedtls_x509_crt_free(&_certificate);
    }
}


void Socket::connect(const string address,
                     unsigned short port)
{
    if (_connected)
        throw logic_error("Can't connect socket that is already connected.");

    int result;

    if ((result = mbedtls_net_connect(&_net_context, address.data(), to_string(port).data(), MBEDTLS_NET_PROTO_UDP)) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_net_connect()", result));

    result = mbedtls_ssl_config_defaults(&_ssl_configuration, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (result != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ssl_config_defaults()", result));

    /* OPTIONAL is usually a bad choice for security, but makes interop easier
     * in this simplified example, in which the ca chain is hardcoded.
     * Production code should set a proper ca chain and use REQUIRED. */
    mbedtls_ssl_conf_authmode(&_ssl_configuration, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&_ssl_configuration, &_certificate, NULL);
    mbedtls_ssl_conf_rng(&_ssl_configuration, mbedtls_ctr_drbg_random, &_ctr_drbg_context);
    mbedtls_ssl_conf_dbg(&_ssl_configuration, simpleDebug, stdout);

    if ((result = mbedtls_ssl_setup(&_ssl_context, &_ssl_configuration)) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ssl_setup()", result));

    if ((result = mbedtls_ssl_set_hostname(&_ssl_context, "localhost")) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ssl_set_hostname()", result));

    mbedtls_ssl_set_bio(&_ssl_context, &_net_context, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
    mbedtls_ssl_set_timer_cb(&_ssl_context, &_delay_context, mbedtls_timing_set_delay, mbedtls_timing_get_delay);

    do
        result = mbedtls_ssl_handshake(&_ssl_context);
    while (result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (result != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ssl_handshake()", result));

    /* In real life, we would have used MBEDTLS_SSL_VERIFY_REQUIRED so that the
     * handshake would not succeed if the peer's cert is bad.  Even if we used
     * MBEDTLS_SSL_VERIFY_OPTIONAL, we would bail out here if ret != 0 */
    uint32_t flags;
    if ((flags = mbedtls_ssl_get_verify_result(&_ssl_context)) != 0)
    {
        char verifying_buffer[512];
        mbedtls_x509_crt_verify_info(verifying_buffer, sizeof(verifying_buffer), "", flags);

        throw runtime_error(verifying_buffer);
    }

    _connected = true;
}

void Socket::close()
{
    if (!_connected)
        return;

    int result;
    do
        result = mbedtls_ssl_close_notify(&_ssl_context);
    while (result == MBEDTLS_ERR_SSL_WANT_WRITE); // TODO: check all possible results

    _connected = false;
}

bool Socket::connected() const
{
    return _connected;
}

size_t Socket::send(const vector<unsigned char>& data)
{
    if (!_connected)
        throw logic_error("Can't send data via closed socket.");

    if (data.size() == 0)
        throw logic_error("Sending data of zero size is not allowed.");

    int bytes_sent;
    do
        bytes_sent = mbedtls_ssl_write(&_ssl_context, data.data(), data.size());
    while (bytes_sent == MBEDTLS_ERR_SSL_WANT_READ || bytes_sent == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (bytes_sent < 0)
    {
        _connected = false;
        throw runtime_error(constructErrorMessage("mbedtls_ssl_write()", bytes_sent));
    }

    return bytes_sent;
}

size_t Socket::receive(vector<unsigned char>& buffer,
                       unsigned long timeout_in_milliseconds)
{
    if (!_connected)
        throw logic_error("Can't receive data from closed socket.");

    _ssl_configuration.read_timeout = timeout_in_milliseconds;

    int bytes_received;
    do
        bytes_received = mbedtls_ssl_read(&_ssl_context, buffer.data(), buffer.size());
    while (bytes_received == MBEDTLS_ERR_SSL_WANT_READ || bytes_received == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (bytes_received >= 0)
    {
        buffer.resize(bytes_received);
        return bytes_received;
    }

    switch (bytes_received)
    {
        case MBEDTLS_ERR_SSL_TIMEOUT:
            buffer.resize(0);
            return 0;

        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
            _connected = false;
            return 0;

        default:
            _connected = false;
            throw runtime_error(constructErrorMessage("mbedtls_ssl_read()", bytes_received));
    }
}

size_t Socket::maximumFragmentSize() const
{
    return mbedtls_ssl_get_max_frag_len(&_ssl_context);;
}

void Socket::generateRandom(unsigned char* buffer,
                            size_t size)
{
    int result;

    if ((result = mbedtls_ctr_drbg_random(&_ctr_drbg_context, buffer, size)) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ctr_drbg_random()", result));
}