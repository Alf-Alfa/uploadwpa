#ifndef HTTPSCLIENT_HPP
#define HTTPSCLIENT_HPP

#include "HTTPClient.hpp"
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

class HTTPSClient : public HTTPClient
{
    public:
        static std::string verifiedCerts;
        bool requireVerification, failedVerification;
        HTTPSClient();
        HTTPSClient(bool requireVerify, int mode);
        virtual ~HTTPSClient();
        static std::unique_ptr<HTTPSClient> makehttps()
        {
            return std::unique_ptr<HTTPSClient>(new HTTPSClient());
        }

        static void initializeSSL();
        void initialize(bool requireVerify, int mode);
        void shutdownSSL();
        virtual bool Connect(const char *host, int port);
        virtual int Read(int maxBytes);
        virtual int Write(char *writeBuffer, int writeSize);
        int Reset();
        template<class T> void Log(T str, bool noNewline);
        template<class T> void Log(T str);
    private:
        static bool sslInitialized;
        BIO *bio;
        SSL *ssl;
        SSL_CTX *ctx;
};

#endif // HTTPSCLIENT_HPP
