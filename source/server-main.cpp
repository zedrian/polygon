#include <iostream>
#include <vector>
#include <iomanip>
#include <memory>

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

#include "socket.h"


using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::endl;
using std::to_string;
using std::hex;
using std::dec;
using std::runtime_error;
using std::exception;
using std::shared_ptr;
using std::make_shared;


/*
 * Acceptor interface :
 *  void listen(address)
 *  shared_ptr<Socket> accept()
 *  void close()
 * */


static void my_debug(void* ctx, int level,
                     const char* file, int line,
                     const char* str)
{
    cout << file << ":" << line << ": " << str << endl;
}

string s_constructErrorMessage(string command,
                             int code)
{
    char buffer[100];
    mbedtls_strerror(code, buffer, 100);

    return command + " failed with error code " + to_string(code) + " - " + buffer;
}


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
size_t maximum_fragment_size;
unsigned char client_ip[16] = {0};
size_t cliip_len;


void initialize()
{
    int ret;

    mbedtls_net_init(&listen_fd);
    mbedtls_net_init(&client_fd);
//    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ssl_cookie_init(&cookie_ctx);
    mbedtls_ssl_cache_init(&cache);
    mbedtls_x509_crt_init(&srvcert);
    mbedtls_pk_init(&pkey);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_debug_set_threshold(0);

    cout << "Loading server certificate and key: ";
    /*
     * This demonstration program uses embedded test certificates.
     * Instead, you may want to use mbedtls_x509_crt_parse_file() to read the
     * server and CA certificates, as well as mbedtls_pk_parse_keyfile().
     */
    ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char*) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len);
    if (ret != 0)
        throw runtime_error(s_constructErrorMessage("mbedtls_x509_crt_parse()", ret));

    ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char*) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (ret != 0)
        throw runtime_error(s_constructErrorMessage("mbedtls_x509_crt_parse()", ret));

    ret = mbedtls_pk_parse_key(&pkey, (const unsigned char*) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0);
    if (ret != 0)
        throw runtime_error(s_constructErrorMessage("mbedtls_pk_parse_key()", ret));

    cout << "success" << endl;


    cout << "Seeding the random number generator: ";
    string personalizating_vector = "dtls_server";
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*) personalizating_vector.data(), personalizating_vector.size())) != 0)
        throw runtime_error(s_constructErrorMessage("mbedtls_ctr_drbg_seed()", ret));
    cout << "success" << endl;

    cout << "Setting up the DTLS data: ";
    if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        throw runtime_error(s_constructErrorMessage("mbedtls_ssl_config_defaults()", ret));

    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);

    mbedtls_ssl_conf_session_cache(&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);

    mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0)
        throw runtime_error(s_constructErrorMessage("mbedtls_ssl_conf_own_cert()", ret));

    if ((ret = mbedtls_ssl_cookie_setup(&cookie_ctx, mbedtls_ctr_drbg_random, &ctr_drbg)) != 0)
        throw runtime_error(s_constructErrorMessage("mbedtls_ssl_cookie_setup()", ret));

    mbedtls_ssl_conf_dtls_cookies(&conf, mbedtls_ssl_cookie_write, mbedtls_ssl_cookie_check, &cookie_ctx);

//    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
//        throw runtime_error(s_constructErrorMessage("mbedtls_ssl_setup()", ret));

//    mbedtls_ssl_set_timer_cb(&ssl, &timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);

    cout << "success" << endl;
}

void bind(string address,
          unsigned short port)
{
    cout << "Binding on UDP: ";

    int ret;
    if ((ret = mbedtls_net_bind(&listen_fd, address.data(), to_string(port).data(), MBEDTLS_NET_PROTO_UDP)) != 0)
        throw runtime_error(s_constructErrorMessage("mbedtls_net_bind()", ret));

    cout << "success" << endl;
}

shared_ptr<Socket> accept()
{
    int ret;

    cout << "Preparing SSL context for remote connection: ";
    mbedtls_ssl_init(&ssl);
    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
        throw runtime_error(s_constructErrorMessage("mbedtls_ssl_setup()", ret));
    mbedtls_ssl_set_timer_cb(&ssl, &timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);
    cout << "success" << endl;

    do
    {
        cout << "Waiting for a remote connection: ";
        mbedtls_net_free(&client_fd);
        mbedtls_ssl_session_reset(&ssl);

        if ((ret = mbedtls_net_accept(&listen_fd, &client_fd, client_ip, sizeof(client_ip), &cliip_len)) != 0)
            throw runtime_error(s_constructErrorMessage("mbedtls_net_accept()", ret));

        /* For HelloVerifyRequest cookies */
        if ((ret = mbedtls_ssl_set_client_transport_id(&ssl, client_ip, cliip_len)) != 0)
            throw runtime_error(s_constructErrorMessage("mbedtls_ssl_set_client_transport_id()", ret));

        mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

        cout << "success" << endl;

        cout << "Performing the DTLS handshake: ";
        do
            ret = mbedtls_ssl_handshake(&ssl);
        while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

        if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED)
        {
            cout << "hello verification requested" << endl;
            continue;
        }
        else if (ret != 0)
        {
            cout << s_constructErrorMessage("mbedtls_ssl_handshake()", ret) << endl;
            continue;
        }

        break;
    }
    while(true);

    cout << "success" << endl;

    cout << "Client address: ";
    for(unsigned char i = 0; i < cliip_len - 1; ++i)
    {
        unsigned short x = client_ip[i];
        cout << dec << x << ".";
    }
    cout << dec << static_cast<unsigned short>(client_ip[cliip_len - 1]) << endl;

    maximum_fragment_size = mbedtls_ssl_get_max_frag_len(&ssl);
    cout << "Maximum size of a fragment for current session: " << maximum_fragment_size << endl;
    return make_shared<Socket>(client_fd, ssl);
}

void work()
{
    size_t bytes_sent, bytes_received;

    vector<unsigned char> buf(1024, 0x00);
    vector<unsigned char> sending_data;

    initialize();

    cout << "Enter listening address: ";
    string listening_address;
    cin >> listening_address;
    cout << "Enter port: ";
    unsigned short port;
    cin >> port;
    bind(listening_address, port);

    do
    {
        auto incoming_socket = accept();

        cout << "Receiving from client: ";
        bytes_received = incoming_socket->receive(buf);
        cout << "success" << endl;

        sending_data = vector<unsigned char>(bytes_received);
        cout << "Received from client (" << dec << bytes_received << " bytes): ";
        for (int i = 0; i < bytes_received; ++i)
        {
            sending_data[i] = buf[i];
            unsigned short x = buf[i];
            cout << hex << x << " ";
        }
        cout << endl;


        cout << "Sending to client: ";
        bytes_sent = incoming_socket->send(sending_data);
        cout << "success" << endl;
        cout << "Sent to client (" << dec << bytes_sent << " bytes): ";
        for (int i = 0; i < bytes_sent; ++i)
        {
            unsigned short x = buf[i];
            cout << hex << x << " ";
        }
        cout << endl;
        cout << "Closing the connection: ";
        incoming_socket->close();
        cout << "success" << endl;
    }
    while(true);
}

void release()
{
    cout << "Release: ";
    mbedtls_net_free(&client_fd);
    mbedtls_net_free(&listen_fd);

    mbedtls_x509_crt_free(&srvcert);
    mbedtls_pk_free(&pkey);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ssl_cookie_free(&cookie_ctx);
    mbedtls_ssl_cache_free(&cache);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    cout << "success" << endl;
}


int main(int argc, char** argv)
{
    try
    {
        work();
    }
    catch (exception& e)
    {
        cout << "fail: " << e.what() << endl;
    }
    release();
}
