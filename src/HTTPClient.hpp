#ifndef HTTPCLIENT_HPP
#define HTTPCLIENT_HPP

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory>

class HTTPClient
{
public:
	int sock, verbosity, responseSize, bufferSize;
    std::unique_ptr<char[]> response;
    std::string requestHeaders, responseHeaders;

    HTTPClient() { verbosity = bufferSize = offset = responseSize = 0; acceptEncoding = "gzip, deflate"; userAgent = "Mozilla/5.0 (X11; Linux x86_64; rv:38.0) Gecko/20100101 Firefox/38.0 Iceweasel/38.6.0"; srand(time(0)); }
    virtual ~HTTPClient() { Close(); }
    static std::unique_ptr<HTTPClient> make()
    {
		return std::unique_ptr<HTTPClient>(new HTTPClient());
	}

	virtual bool Connect(const char *host, int port);
	bool Get(std::string page);
	bool Post(std::string page, std::string &data);
	bool PostMultiPart(std::string page, std::string &data, std::string boundary);
	bool WriteRequestReadResponse();
	size_t interpretResponse();
	virtual void clearResponse();
	virtual int Read(int maxBytes);
	virtual int Write(char *writeBuffer, int writeSize);
	static std::string getRandomBoundary();
	static std::string urlEncode(std::string str);
	static std::string urlDecode(std::string str);
	template<class T> void Log(T str, bool noNewline);
	template<class T> void Log(T str);
	void Close() { close(sock); }
	//------------------------------------------------------------------------------
	std::string getUserAgent() { return userAgent; }
    void setUserAgent(std::string val) { if(!val.empty()) { userAgent = val; } }
    std::string getAcceptEncoding() { return acceptEncoding; }
    void setAcceptEncoding(std::string val) { if(!val.empty()) { acceptEncoding = val; } }
    std::string getHost() { return host; }
    void setHost(std::string val) { host = val; }
    std::string getReferer() { return referer; }
    void setReferer(std::string val) { referer = val; }
    int getVerbosity() { return verbosity; }
    void setVerbosity(int val) { verbosity = val; }
	int getPort() { return port; }
	void setPort(int val) { port = val; }
private:
    struct hostent *server;
	struct sockaddr_in serverAddress;
	size_t offset, dataStartOffset;
    int port;
    std::string host, request, referer, userAgent, acceptEncoding;
};

#endif
