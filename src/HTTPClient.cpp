#include "HTTPClient.hpp"

bool HTTPClient::Connect(const char *host, int port)
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		Log("ERROR opening socket");
		return false;
	}

	server = gethostbyname(host);
	if (server == 0)
	{
		Log("ERROR no such host");
		return false;
	}

	bzero((char*)&serverAddress, sizeof(serverAddress));
	bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);

	if(connect(sock,(struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		Log("ERROR connecting");
		return false;
	}

	this->host = host;
	this->port = port;
	setReferer("http://" + this->host + "/"); //default referer
	return true;
}

bool HTTPClient::Get(const char *page)
{
    if(!page) return false;

	request = "GET ";
	request += page;
	request += " HTTP/1.1\r\n";
	if(!host.empty())
		request += "Host: " + host + "\r\n";
	request += userAgent + "\r\n";
	request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
	request += "Accept-Language: en-US,en;q=0.5\r\n";
	request += "Accept-Encoding: gzip, deflate\r\n";
	if(!referer.empty())
		request += "Referer: " + referer + "\r\n";
	request += "Connection: keep-alive\r\n\r\n";
	requestHeaders = request;
	Log("\n[Request: ]");
	Log(request, true);

	return WriteRequestReadResponse();
}

bool HTTPClient::Post(const char *page, const char *data)
{
    if(!page || !data) return false;

    request = "POST ";
    request += page;
    request += " HTTP/1.1\r\n";
    if(!host.empty())
		request += "Host: " + host + "\r\n";
	request += userAgent + "\r\n";
	request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
	request += "Accept-Language: en-US,en;q=0.5\r\n";
	request += "Accept-Encoding: identity\r\n"; //gzip, deflate
	if(!referer.empty())
		request += "Referer: " + referer + "\r\n";
	request += "Connection: keep-alive\r\n";
	request += "Content-Type: application/x-www-form-urlencoded\r\n";
	request += "Content-Length: ";
	char dataLenStr[21]{0};
    sprintf(dataLenStr, "%lu", strlen(data));
	request += dataLenStr;
	request += "\r\n\r\n";
	requestHeaders = request;
	request += data;
	request += "\r\n\r\n";
	Log("\n[Request: ]");
	Log(request, true);

	return WriteRequestReadResponse();
}

bool HTTPClient::PostMultiPart(const char *page, std::string &data, std::string &boundary)
{
    if(!page || data.empty()) return false;

    request = "POST ";
    request += page;
    request += " HTTP/1.1\r\n";
    if(!host.empty())
		request += "Host: " + host + "\r\n";
	request += userAgent + "\r\n";
	request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
	request += "Accept-Language: en-US,en;q=0.5\r\n";
	request += "Accept-Encoding: identity\r\n";
	if(!referer.empty())
		request += "Referer: " + referer + "\r\n";
	request += "Connection: keep-alive\r\n";
	request += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
	request += "Content-Length: ";
	char dataLenStr[21]{0};
    sprintf(dataLenStr, "%lu", data.size());
	request += dataLenStr;
	request += "\r\n\r\n";
	requestHeaders = request;
	Log("\n[Request: ]");
	Log(request.c_str(), true);

	request += data;
	request += "\r\n\r\n";

	return WriteRequestReadResponse();
}

bool HTTPClient::WriteRequestReadResponse()
{
    int wroteNum = Write((char*)request.c_str(), request.size());
	if(wroteNum > 0)
	{
		int readNum = Read(4096);
		if(readNum > 0)
		{
            interpretResponse();
            return true;
        }
	}
	return false;
}

size_t HTTPClient::interpretResponse()
{
    char *startOfHeader = response.get();
    char *endOfHeader = strstr(startOfHeader, "\r\n\r\n");
    if(endOfHeader == 0) return 0;

    size_t headerSize = (endOfHeader - startOfHeader) + 4;
    responseHeaders.resize(headerSize);
    bcopy(response.get(), (void*)responseHeaders.data(), headerSize);
    offset = headerSize;

    Log("\n[Response Headers: ]"); //more work needs to be done here, to make sure to get the whole response
    Log(responseHeaders.c_str());  //eg. get the content length and however many bytes recieved of it so far, then get the rest if necessary.

    return headerSize;
}

int HTTPClient::Read(int maxBytes)
{
	if(!response.get() || (bufferSize < maxBytes))
	{
		response = std::unique_ptr<char[]>(new char[maxBytes]);
		bufferSize = maxBytes;
	}
	if(!response.get()) return -1;
	bzero(response.get(), bufferSize);

	Log("recieving...");
	int num = recv(sock, response.get(), maxBytes, 0);
	if(num <= 0)
	{
		Log("ERROR reading from socket");
		return 0;
	}

    responseSize = num;
	return num;
}

int HTTPClient::Write(char *writeBuffer, int writeSize)
{
    int num, total = 0, bytesLeft = writeSize;

    while(total < writeSize)
    {
        num = send(sock, writeBuffer+total, bytesLeft, 0);
        if(num <= 0) break;
        total += num;
        bytesLeft -= num;
    }

	if(total < writeSize)
	{
		Log("ERROR writing to socket");
		return 0;
	}

	return total;
}

std::string HTTPClient::getRandomBoundary()
{
    std::string randomBoundary = "---------------------------";
    {
        uint64_t random128bits[2];
        FILE *randomness = fopen("/dev/urandom", "rb");
        if(randomness)
        {
            fread(&random128bits[0], 16, 1, randomness);
            fclose(randomness);
            for(int i = 0; i < 2; i++)
            {
                char randomNum[30];
                sprintf(randomNum,"%lu",random128bits[i]);
                randomBoundary += randomNum;
            }

            srand(time(0));
            int losehowmanydigits = rand() % 3 + 11;
            randomBoundary.resize(randomBoundary.size()-losehowmanydigits);
            //resulting in a 23 - 29 digit number appended to 27 dashes (like how iceweasal does it)
            return randomBoundary;
        }
    }
    return "";
}

std::string HTTPClient::urlEncode(std::string str)
{
    std::string new_str = "";
    char c;
    int ic;
    const char* chars = str.c_str();
    char bufHex[10];
    int len = strlen(chars);

    for(int i=0;i<len;i++)
    {
        c = chars[i];
        ic = c;
        // uncomment this if you want to encode spaces with +
        /*if (c==' ') new_str += '+';
        else */
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') new_str += c;
        else
        {
            sprintf(bufHex,"%X",c);
            if(ic < 16)
                new_str += "%0";
            else
                new_str += "%";
            new_str += bufHex;
        }
    }
    return new_str;
 }

std::string HTTPClient::urlDecode(std::string str)
{
    std::string ret;
    char ch;
    int i, ii, len = str.length();

    for (i=0; i < len; i++)
    {
        if(str[i] != '%')
        {
            if(str[i] == '+')
                ret += ' ';
            else
                ret += str[i];
        }
        else
        {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
    }
    return ret;
}

template<class T> void HTTPClient::Log(T str, bool noNewline)
{
    if(verbosity > 0)
    {
        std::cout << str;
        if(!noNewline) std::cout << "\n";
    }
}
