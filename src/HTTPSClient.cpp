#include "HTTPSClient.hpp"

bool HTTPSClient::sslInitialized = false;
std::string HTTPSClient::verifiedCerts;

int verify(int ok, X509_STORE_CTX *store)
{
    if(ok > 0)
        HTTPSClient::verifiedCerts += "[OK] ";
    else
        HTTPSClient::verifiedCerts += "[FAILED] ";
    HTTPSClient::verifiedCerts += "Name: ";
    HTTPSClient::verifiedCerts += store->current_cert->name;
    HTTPSClient::verifiedCerts += "\n";

    int depth = X509_STORE_CTX_get_error_depth(store);

    //X509* cert = X509_STORE_CTX_get_current_cert(store);
    //X509_NAME* iname = cert ? X509_get_issuer_name(cert) : NULL;
    //X509_NAME* sname = cert ? X509_get_subject_name(cert) : NULL;

    //print_cn_name("Issuer (cn)", iname);
    //print_cn_name("Subject (cn)", sname);

    if(depth == 0)
    {
        /* If depth is 0, its the server's certificate. Print the SANs too */
        //print_san_name("Subject (san)", cert);
    }

    return ok;
}

HTTPSClient::HTTPSClient()
{
    initialize(true, 23);
}

HTTPSClient::HTTPSClient(bool requireVerify, int mode = 3)
{
    initialize(requireVerify, mode);
}

void HTTPSClient::initialize(bool requireVerify, int mode)
{
    if(!sslInitialized)
        initializeSSL();
    if(mode == 2 || mode == 23)
        ctx = SSL_CTX_new(SSLv23_client_method());
    else
        ctx = SSL_CTX_new(SSLv3_client_method());

    requireVerification = requireVerify;

    SSL_CTX_set_verify_depth(ctx, 4);
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
    SSL_CTX_set_default_verify_paths(ctx);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify);
}

HTTPSClient::~HTTPSClient()
{
    //shutdownSSL();
    BIO_free_all(bio);
    SSL_CTX_free(ctx);
}

void HTTPSClient::initializeSSL()
{
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    sslInitialized = true;
}

void HTTPSClient::shutdownSSL()
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

bool HTTPSClient::Connect(const char *host, int port)
{
    verifiedCerts.clear();

    bio = BIO_new_ssl_connect(ctx);
    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    std::string hostport = std::string(host) + ":" + std::to_string(port);

    Log("Connection on hostport: " + hostport);
    BIO_set_conn_hostname(bio, hostport.c_str());
    /* Verify the connection opened and perform the handshake */
    if(BIO_do_connect(bio) <= 0)
    {
        Log("TLS Connection Failed...");
        return false;
    }

    if(BIO_do_handshake(bio) <= 0)
    {
        Log("TLS Handshake Failed...");
        return false;
    }

    X509 *cert = SSL_get_peer_certificate(ssl);
    if(cert) { X509_free(cert); }
    if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
        Log("Failed verification...");
        failedVerification = true;
        if(requireVerification) { Reset(); return false; }
    }
    else
    {
        Log(verifiedCerts.c_str(), true);
        Log("Successfully verified!");
        failedVerification = false;
    }
    setHost(host);
	setPort(port);
    setReferer("https://" + getHost() + "/"); //default referer
    return true;
}

int HTTPSClient::Read(int maxBytes)
{
    size_t offset = 0;
    if(!response.get() || (bufferSize < maxBytes))
	{
        std::unique_ptr<char[]> oldResponse;
        if(response.get())
        {
            oldResponse = std::move(response);
            maxBytes += responseSize;
        }
        response = std::unique_ptr<char[]>(new char[maxBytes]);
        bufferSize = maxBytes;
        offset = responseSize;
        bcopy(oldResponse.get(), response.get(), responseSize);
    }
	if(!response.get()) return 0;

    Log("ssl receiving...");
	int num = BIO_read(bio, &response.get()[offset], (maxBytes - offset));
    if(num <= 0)
    {
        Log("SSL read failed...");
        return 0;
    }

    responseSize += num;
    Log("Recieved over SSL... #bytes: " + std::to_string(num));
	return num;
}

int HTTPSClient::Write(char *writeBuffer, int writeSize)
{
    if(BIO_write(bio, writeBuffer, writeSize) <= 0)
    {
        if(!BIO_should_retry(bio))
        {
            Log("SSL write failed... shouldn't retry");
            return 0;
        }
        Log("SSL write failed...");
        return 0;
    }

	Log("Sent over SSL... #bytes: " + std::to_string(writeSize));
	return writeSize;
}

int HTTPSClient::Reset()
{
    return BIO_reset(bio);
}

template<class T> void HTTPSClient::Log(T Str, bool noNewline)
{
    if(verbosity > 0)
    {
        std::cout << Str;
        if(!noNewline) std::cout << "\n";
    }
}

template<class T> void HTTPSClient::Log(T Str)
{
    Log(Str, false);
}
