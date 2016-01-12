#include <iomanip>
#include <iostream>
#include <vector>

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
using std::runtime_error;
using std::logic_error;
using std::exception;


/*
 * Socket interface :
 *  size_t& sendTimeout() ?
 *  size_t& receiveTimeout() ?
 *  size_t send(const unsigned char* data, size_t size)
 *  size_t receive(unsigned char* data, size_t maximum_size)
 *  size_t pendingDatagramSize() ?
 *  bool hasPendingDatagams() ?
 */


static void my_debug(void *ctx, int level,
                     const char *file, int line,
                     const char *str)
{
    cout << file << ":" << line << ": " << str << endl;
}

string constructErrorMessage(string command,
                             int code)
{
    char buffer[100];
    mbedtls_strerror(code, buffer, 100);

    return command + " failed with error code " + to_string(code) + " - " + buffer;
}

mbedtls_net_context server_fd;
mbedtls_x509_crt cacert;
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
size_t maximum_fragment_size;
bool active;


void initialize()
{
    int ret;
    mbedtls_debug_set_threshold(0);

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
    string personalizating_vector = "dtls_client";
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) personalizating_vector.data(),
                                     personalizating_vector.size())) != 0)
        throw runtime_error(constructErrorMessage("mbedtls_ctr_drbg_seed()", ret));

    cout << "success" << endl;

    /*
     * 0. Load certificates
     */
    cout << "Loading the CA root certificate: ";

    ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (ret < 0)
        throw runtime_error(constructErrorMessage("mbedtls_x509_crt_parse()", ret));

    active = false;
    cout << "success (" << ret << " skipped)" << endl;
}

void connect(const string address,
             unsigned int port)
{
    int ret;
    cout << "Connecting to server: ";

    if ((ret = mbedtls_net_connect(&server_fd, address.data(), to_string(port).data(), MBEDTLS_NET_PROTO_UDP)) != 0)
    {
        throw runtime_error(constructErrorMessage("mbedtls_net_connect()", ret));
    }

    cout << "success" << endl;

    /*
     * 2. Setup stuff
     */
    cout << "Setting up the DTLS structure: ";

    ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0)
    {
        throw runtime_error(constructErrorMessage("mbedtls_ssl_config_defaults()", ret));
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
        throw runtime_error(constructErrorMessage("mbedtls_ssl_setup()", ret));
    }

    if ((ret = mbedtls_ssl_set_hostname(&ssl, "localhost")) != 0)
    {
        throw runtime_error(constructErrorMessage("mbedtls_ssl_set_hostname()", ret));
    }

    mbedtls_timing_delay_context timer;
    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
    mbedtls_ssl_set_timer_cb(&ssl, &timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);

    cout << "success" << endl;

    /*
     * 4. Handshake
     */
    cout << "Performing the SSL/TLS handshake: ";

    do
    {
        ret = mbedtls_ssl_handshake(&ssl);
    }
    while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (ret != 0)
    {
        throw runtime_error(constructErrorMessage("mbedtls_ssl_handshake()", ret));
    }

    cout << "success" << endl;

    /*
     * 5. Verify the server certificate
     */
    cout << "Verifying peer X.509 certificate: ";

    /* In real life, we would have used MBEDTLS_SSL_VERIFY_REQUIRED so that the
     * handshake would not succeed if the peer's cert is bad.  Even if we used
     * MBEDTLS_SSL_VERIFY_OPTIONAL, we would bail out here if ret != 0 */
    uint32_t flags;
    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
        char vrfy_buf[512];
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "", flags);

        throw runtime_error(vrfy_buf);
    }

    active = true;
    cout << "success" << endl;

    maximum_fragment_size = mbedtls_ssl_get_max_frag_len(&ssl);
    cout << "Maximum size of a fragment for current session: " << maximum_fragment_size << endl;
}

size_t receive(unsigned char *data,
               size_t maximum_size) // TODO: add something like a timeout
{
    if(data == nullptr)
        throw logic_error("Passed a nullptr to receive().");

    int bytes_received;

    do
        bytes_received = mbedtls_ssl_read(&ssl, data, maximum_size);
    while (bytes_received == MBEDTLS_ERR_SSL_WANT_READ || bytes_received == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (bytes_received > 0)
        return bytes_received;

    switch (bytes_received)
    {
        case MBEDTLS_ERR_SSL_TIMEOUT:
            cout << "timeout" << endl;
            return 0;

        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
            cout << "connection was closed gracefully" << endl;
            active = false;
            return 0;

        default:
            throw runtime_error(constructErrorMessage("mbedtls_ssl_read()", bytes_received));
    }
}

size_t send(const unsigned char *data,
            size_t size)
{
    if(data == nullptr)
        throw logic_error("Passed a nullptr to send().");

    if (size > maximum_fragment_size)
        throw logic_error("Sending data bigger than maximum fragment size is not supported.");

    int bytes_sent;
    do
        bytes_sent = mbedtls_ssl_write(&ssl, data, size);
    while (bytes_sent == MBEDTLS_ERR_SSL_WANT_READ || bytes_sent == MBEDTLS_ERR_SSL_WANT_WRITE);

    if (bytes_sent < 0)
        throw runtime_error(constructErrorMessage("mbedtls_ssl_write()", bytes_sent));

    return bytes_sent;
}

vector<unsigned char> sendWithConfirmation(const vector<unsigned char> &data)
{
    vector<unsigned char> response(maximum_fragment_size, 0x00);
    size_t bytes_received;

    while (true)
    {
        cout << "Sending to server: ";
        send(data.data(), data.size());
        cout << "success" << endl;

        cout << "Receiving confirmation from server: ";
        bytes_received = receive(response.data(), response.size());

        if (bytes_received > 0)
            break;

        if (!active)
            return vector<unsigned char>();
    }
    cout << "success" << endl;

    response.resize(bytes_received);
    return response;
}


void close()
{
    int ret;

    /* No error checking, the connection might be closed already */
    do
        ret = mbedtls_ssl_close_notify(&ssl);
    while (ret == MBEDTLS_ERR_SSL_WANT_WRITE); // TODO: check all possible results

    active = false;
}

void work()
{
    initialize();

    string server_address;
    unsigned int port;
    cout << "Enter server IP: ";
    cin >> server_address;
    cout << "Enter server port: ";
    cin >> port;

    connect(server_address, port);

    vector<unsigned char> data(10, 0x00);
    for (unsigned char i = 0; i < 10; ++i)
        data[i] = i + i * 0x10;

    cout << "Data to send to server (" << dec << data.size() << " bytes): ";
    for (int i = 0; i < data.size(); ++i)
    {
        unsigned short x = data[i];
        cout << hex << x << " ";
    }
    cout << endl;

    auto response = sendWithConfirmation(data);

    cout << "Server's response (" << dec << response.size() << " bytes): ";
    for (int i = 0; i < response.size(); ++i)
    {
        unsigned short x = response[i];
        cout << hex << x << " ";
    }
    cout << endl;

    cout << "Closing the connection: ";
    close();
    cout << "success" << endl;
}

void release()
{
    cout << "Release: ";
    mbedtls_net_free(&server_fd);

    mbedtls_x509_crt_free(&cacert);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    cout << "success" << endl;
}


int main(int argc, char *argv[])
{
    try
    {
        work();
    }
    catch (exception &e)
    {
        cout << "fail: " << e.what() << endl;
    }
    release();
}