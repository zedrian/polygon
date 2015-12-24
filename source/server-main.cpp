#include <iostream>
#include <vector>
#include <iomanip>

#include <mbedtls/config.h>
#include <mbedtls/net.h>
#include <mbedtls/ssl_cookie.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/timing.h>
#include <mbedtls/ssl_cache.h>
#include <mbedtls/debug.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>


using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::endl;
using std::to_string;
using std::hex;
using std::dec;


static void my_debug(void* ctx, int level,
                     const char* file, int line,
                     const char* str)
{
    cout << file << ":" << line << ": " << str << endl;
}

int main(int argc, char** argv)
{
    int ret, len;
    mbedtls_net_context listen_fd, client_fd;
    vector<unsigned char> buf(1024, 0x00);
    string personalizating_vector = "dtls_server";
    unsigned char client_ip[16] = {0};
    size_t cliip_len;
    mbedtls_ssl_cookie_ctx cookie_ctx;
    string listening_address;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt srvcert;
    mbedtls_pk_context pkey;
    mbedtls_timing_delay_context timer;
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_context cache;
#endif

    mbedtls_net_init(&listen_fd);
    mbedtls_net_init(&client_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ssl_cookie_init(&cookie_ctx);
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_init(&cache);
#endif
    mbedtls_x509_crt_init(&srvcert);
    mbedtls_pk_init(&pkey);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(0);
#endif

    /*
     * 1. Load the certificates and private RSA key
     */
    cout << "Loading server certificate and key: ";

    /*
     * This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char*) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len);
    if (ret != 0)
    {
        cout << "failed: mbedtls_x509_crt_parse() returned " << ret << endl;
        goto exit;
    }

    ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char*) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (ret != 0)
    {
        cout << "failed: mbedtls_x509_crt_parse() returned " << ret << endl;
        goto exit;
    }

    ret = mbedtls_pk_parse_key(&pkey, (const unsigned char*) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0);
    if (ret != 0)
    {
        cout << "failed: mbedtls_pk_parse_key() returned " << ret << endl;
        goto exit;
    }

    cout << "success" << endl;

    /*
     * 2. Setup the "listening" UDP socket
     */
    cout << "Enter listening address: ";
    cin >> listening_address;
    cout << "Enter port: ";
    unsigned int port;
    cin >> port;
    cout << "Binding on UDP: ";

    if ((ret = mbedtls_net_bind(&listen_fd, listening_address.data(), to_string(port).data(), MBEDTLS_NET_PROTO_UDP)) != 0)
    {
        cout << "failed: mbedtls_net_bind() returned " << ret << endl;
        goto exit;
    }

    cout << "success" << endl;

    /*
     * 3. Seed the RNG
     */
    cout << "Seeding the random number generator: ";

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*) personalizating_vector.data(), personalizating_vector.size())) != 0)
    {
        cout << "failed: mbedtls_ctr_drbg_seed() returned " << ret << endl;
        goto exit;
    }

    cout << "success" << endl;

    /*
     * 4. Setup stuff
     */
    cout << "Setting up the DTLS data: ";

    if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        cout << "failed: mbedtls_ssl_config_defaults() returned " << ret << endl;
        goto exit;
    }

    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);

#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_conf_session_cache(&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
#endif

    mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0)
    {
        cout << "failed: mbedtls_ssl_conf_own_cert() returned " << ret << endl;
        goto exit;
    }

    if ((ret = mbedtls_ssl_cookie_setup(&cookie_ctx, mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
    {
        cout << "failed: mbedtls_ssl_cookie_setup() returned " << ret << endl;
        goto exit;
    }

    mbedtls_ssl_conf_dtls_cookies(&conf, mbedtls_ssl_cookie_write, mbedtls_ssl_cookie_check, &cookie_ctx);

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        cout << "failed: mbedtls_ssl_setup() returned " << ret << endl;
        goto exit;
    }

    mbedtls_ssl_set_timer_cb(&ssl, &timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);

    cout << "success" << endl;

    reset:
#ifdef MBEDTLS_ERROR_C
    if (ret != 0)
    {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, 100);
        cout << "Last error was: " << ret << " - " << error_buf << endl;
    }
#endif

    mbedtls_net_free(&client_fd);

    mbedtls_ssl_session_reset(&ssl);

    /*
     * 3. Wait until a client connects
     */
    cout << "Waiting for a remote connection: ";

    if ((ret = mbedtls_net_accept(&listen_fd, &client_fd, client_ip, sizeof(client_ip), &cliip_len)) != 0)
    {
        cout << "failed: mbedtls_net_accept() returned " << ret << endl;
        goto exit;
    }

    /* For HelloVerifyRequest cookies */
    if ((ret = mbedtls_ssl_set_client_transport_id(&ssl, client_ip, cliip_len)) != 0)
    {
        cout << "failed: mbedtls_ssl_set_client_transport_id() returned " << ret << endl;
        goto exit;
    }

    mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    cout << "success" << endl;

    /*
     * 5. Handshake
     */
    cout << "Performing the DTLS handshake: ";

    do
        ret = mbedtls_ssl_handshake(&ssl);
    while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED)
    {
        cout << "hello verification requested" << endl;
        ret = 0;
        goto reset;
    }
    else if (ret != 0)
    {
        cout << "failed: mbedtls_ssl_handshake() returned " << ret << endl;
        goto reset;
    }

    cout << "success" << endl;

    /*
     * 6. Read the echo Request
     */
    cout << "Receiving from client: ";

    len = buf.size() - 1;

    do
        ret = mbedtls_ssl_read(&ssl, buf.data(), len);
    while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret <= 0)
    {
        switch (ret)
        {
            case MBEDTLS_ERR_SSL_TIMEOUT:
                cout << "timeout" << endl;
                goto reset;

            case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                cout << "connection was closed gracefully" << endl;
                ret = 0;
                goto close_notify;

            default:
                cout << "mbedtls_ssl_read() returned " << ret << endl;
                goto reset;
        }
    }
    cout << "success" << endl;

    len = ret;
    cout << "Received from client (" << dec << len << " bytes): ";
    for (int i = 0; i < len; ++i)
    {
        unsigned short x = buf[i];
        cout << hex << x << " ";
    }
    cout << endl;

    /*
     * 7. Write the 200 Response
     */
    cout << "Sending to client: ";

    do
        ret = mbedtls_ssl_write(&ssl, buf.data(), len);
    while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret < 0)
    {
        cout << "failed: mbedtls_ssl_write() returned " << ret << endl;
        goto exit;
    }
    cout << "success" << endl;

    len = ret;
    cout << "Sent to client (" << dec << len << " bytes): ";
    for (int i = 0; i < len; ++i)
    {
        unsigned short x = buf[i];
        cout << hex << x << " ";
    }
    cout << endl;

    /*
     * 8. Done, cleanly close the connection
     */
    close_notify:
    cout << "Closing the connection: ";

    /* No error checking, the connection might be closed already */
    do ret = mbedtls_ssl_close_notify(&ssl);
    while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);
    ret = 0;

    cout << "success" << endl;

    goto reset;

    /*
     * Final clean-ups and exit
     */
    exit:

#ifdef MBEDTLS_ERROR_C
    if (ret != 0)
    {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, 100);
        cout << "Last error was: " << ret << " - " << error_buf << endl;
    }
#endif

    mbedtls_net_free(&client_fd);
    mbedtls_net_free(&listen_fd);

    mbedtls_x509_crt_free(&srvcert);
    mbedtls_pk_free(&pkey);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ssl_cookie_free(&cookie_ctx);
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_free(&cache);
#endif
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

#if defined(_WIN32)
    cout << "Press Enter to exit this program." << endl;
    fflush(stdout);
    getchar();
#endif

    /* Shell can not handle large exit numbers -> 1 for errors */
    if (ret < 0)
        ret = 1;

    return (ret);
}