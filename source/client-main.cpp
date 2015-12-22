#include <iomanip>
#include <iostream>
#include <vector>

#include <mbedtls/platform.h>
#include <mbedtls/net.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/timing.h>
#include <mbedtls/debug.h>
#include <mbedtls/certs.h>
#include <mbedtls/error.h>


using std::cin;
using std::cout;
using std::endl;
using std::to_string;
using std::string;
using std::vector;
using std::hex;
using std::dec;


static void my_debug(void* ctx, int level,
                     const char* file, int line,
                     const char* str)
{
    cout << file << ":" << line << ": " << str << endl;
}

int main(int argc, char* argv[])
{
    int ret, len;
    mbedtls_net_context server_fd;
    uint32_t flags;
    vector<unsigned char> buf(1024, 0x00);
    string personalizating_vector = "dtls_client";
    int retry_left = 5;
    string server_address;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_timing_delay_context timer;


#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(0);
#endif

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_net_init(&server_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    cout << "Seeding the random number generator: ";

    mbedtls_entropy_init(&entropy);
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*) personalizating_vector.data(), personalizating_vector.size())) != 0)
    {
        cout << "failed: mbedtls_ctr_drbg_seed() returned " << ret << endl;
        goto exit;
    }

    cout << "success" << endl;

    /*
     * 0. Load certificates
     */
    cout << "Loading the CA root certificate: ";

    ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char*) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (ret < 0)
    {
        cout << "failed: mbedtls_x509_crt_parse() returned " << ret << endl;
        goto exit;
    }

    cout << "success (" << ret << " skipped)" << endl;

    /*
     * 1. Start the connection
     */
    cout << "Enter server IP: ";
    cin >> server_address;
    cout << "Enter server port: ";
    unsigned int port;
    cin >> port;
    cout << "Connecting to server: ";

    if ((ret = mbedtls_net_connect(&server_fd, server_address.data(), to_string(port).data(), MBEDTLS_NET_PROTO_UDP)) != 0)
    {
        cout << "failed: mbedtls_net_connect() returned " << ret << endl;
        goto exit;
    }

    cout << "success" << endl;

    /*
     * 2. Setup stuff
     */
    cout << "Setting up the DTLS structure: ";

    if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        cout << "failed: mbedtls_ssl_config_defaults() returned " << ret << endl;
        goto exit;
    }

    /* OPTIONAL is usually a bad choice for security, but makes interop easier
     * in this simplified example, in which the ca chain is hardcoded.
     * Production code should set a proper ca chain and use REQUIRED. */
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        cout << "failed: mbedtls_ssl_setup() returned " << ret << endl;
        goto exit;
    }

    if ((ret = mbedtls_ssl_set_hostname(&ssl, "localhost")) != 0)
    {
        cout << "failed: mbedtls_ssl_set_hostname() returned " << ret << endl;
        goto exit;
    }

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    mbedtls_ssl_set_timer_cb(&ssl, &timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);

    cout << "success" << endl;

    /*
     * 4. Handshake
     */
    cout << "Performing the SSL/TLS handshake: ";

    do
        ret = mbedtls_ssl_handshake(&ssl);
    while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret != 0)
    {
        cout << "failed: mbedtls_ssl_handshake() returned " << ret << endl;
        goto exit;
    }

    cout << "success" << endl;

    /*
     * 5. Verify the server certificate
     */
    cout << "Verifying peer X.509 certificate: ";

    /* In real life, we would have used MBEDTLS_SSL_VERIFY_REQUIRED so that the
     * handshake would not succeed if the peer's cert is bad.  Even if we used
     * MBEDTLS_SSL_VERIFY_OPTIONAL, we would bail out here if ret != 0 */
    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
        char vrfy_buf[512];

        cout << "failed: ";

        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "", flags);

        cout << vrfy_buf << endl;
    }
    else
        cout << "success" << endl;

    /*
     * 6. Write the echo request
     */
    send_request:
    cout << "Sending to server: ";
    for(unsigned char i  = 0; i < 10; ++i)
        buf[i] = i + i*0x10;

    len = 10;

    do
        ret = mbedtls_ssl_write(&ssl, buf.data(), len);
    while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret < 0)
    {
        cout << "failed: mbedtls_ssl_write() returned " << ret << endl;
        goto exit;
    }

    len = ret;
    cout << "Sent to server (" << len << " bytes): ";
    for (int i = 0; i < len; ++i)
    {
        unsigned short x = buf[i];
        cout << hex << x << " ";
    }
    cout << endl;

    /*
     * 7. Read the echo response
     */
    cout << "Receiving from server: ";

    len = sizeof(buf) - 1;
    buf = vector<unsigned char>(len, 0x00);

    do
        ret = mbedtls_ssl_read(&ssl, buf.data(), len);
    while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret <= 0)
    {
        switch (ret)
        {
            case MBEDTLS_ERR_SSL_TIMEOUT:
                cout << "timeout" << endl;
                if (retry_left-- > 0)
                    goto send_request;
                goto exit;

            case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                cout << "connection was closed gracefully" << endl;
                ret = 0;
                goto close_notify;

            default:
                cout << "mbedtls_ssl_read() returned " << ret << endl;
                goto exit;
        }
    }

    len = ret;
    cout << "Received from server (" << dec << len << " bytes): ";
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

    /*
     * 9. Final clean-ups and exit
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

    mbedtls_net_free(&server_fd);

    mbedtls_x509_crt_free(&cacert);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

#if defined(_WIN32)
    cout << "Press Enter to exit" << endl;
    getchar();
#endif

    /* Shell can not handle large exit numbers -> 1 for errors */
    if (ret < 0)
        ret = 1;

    return (ret);
}